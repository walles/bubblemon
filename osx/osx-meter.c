//
//  osx-meter.c
//  bubblemon
//
//  Created by Johan Walles on 2012-04-29.
//  Copyright (c) 2012 johan.walles@gmail.com. All rights reserved.
//

#include "dynamic-accumulator.h"
#include "mail.h"
#include "meter.h"

#include <assert.h>
#include <mach/host_info.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <stdlib.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOBSD.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/ps/IOPSKeys.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/storage/IOBlockStorageDriver.h>
#include <IOKit/storage/IOMedia.h>

static mach_port_t masterPort;

// How large is the CPU load history?  Large values gives good precision
// but over a long time.  Small values gives worse precision but more
// current values.
#define LOADSAMPLES 16

// Measure load every N calls.  The measurement itself can be costly;
// this gets our CPU usage down
#define MEASURE_LOAD_EVERY 5

static void measureSwap(meter_sysload_t *load) {
  int vmmib[2] = {CTL_VM, VM_SWAPUSAGE};
  struct xsw_usage swapused; /* defined in sysctl.h */
  size_t swlen = sizeof(swapused);
  assert(sysctl(vmmib, 2, &swapused, &swlen, NULL, 0) >= 0);

  load->swapUsed = swapused.xsu_used;
  load->swapSize = swapused.xsu_total;
}

static void measureRam(meter_sysload_t *load) {
  mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;

  vm_statistics64_data_t vmstat;
  assert(host_statistics64(mach_host_self(), HOST_VM_INFO64,
                           (host_info64_t)&vmstat, &count) == KERN_SUCCESS);

  // This matches what the Activity Monitor shows in macOS 10.15.6
  //
  // For internal - purgeable: https://stackoverflow.com/a/36721309/473672
  size_t usedPages = (vmstat.internal_page_count - vmstat.purgeable_count) +
                     vmstat.wire_count + vmstat.compressor_page_count;
  size_t pageSize = sysconf(_SC_PAGESIZE);

  load->memoryUsed = usedPages * pageSize;

  // Get the Physical memory size, from:
  // https://stackoverflow.com/a/1909988/473672
  int mib[2] = {CTL_HW, HW_MEMSIZE};
  int64_t physical_memory;
  size_t length;
  length = sizeof(physical_memory);
  int sysctlResult = sysctl(mib, 2 /* mib array element count */,
                            &physical_memory, &length, NULL, 0);
  assert(sysctlResult == 0);

  load->memorySize = physical_memory;
}

static int getBatteryChargePercent(void) {
  int computerChargePercent = 100;

  CFArrayRef powerSourcesList = NULL;
  CFTypeRef powerSourcesInfo = IOPSCopyPowerSourcesInfo();
  if (!powerSourcesInfo) {
    // No battery info, assume external power source
    goto done;
  }

  powerSourcesList = IOPSCopyPowerSourcesList(powerSourcesInfo);
  if (!powerSourcesList) {
    // No battery info, assume external power source
    goto done;
  }

  long count = CFArrayGetCount(powerSourcesList);
  for (int i = 0; i < count; i++) {
    CFDictionaryRef powerSource = IOPSGetPowerSourceDescription(
        powerSourcesInfo, CFArrayGetValueAtIndex(powerSourcesList, i));
    if (powerSource == NULL) {
      // Bubble trouble, try the next one
      continue;
    }

    CFBooleanRef isChargingRef =
        CFDictionaryGetValue(powerSource, CFSTR(kIOPSIsChargingKey));
    if (isChargingRef == NULL) {
      continue;
    }
    Boolean isCharging = CFBooleanGetValue(isChargingRef);
    if (isCharging) {
      // No need to bring the user's attention to this one; we only want to
      // look powerless if they risk running out.
      continue;
    }

    CFNumberRef chargeRef =
        CFDictionaryGetValue(powerSource, CFSTR(kIOPSCurrentCapacityKey));
    if (chargeRef == NULL) {
      continue;
    }
    CFNumberRef capacityRef =
        CFDictionaryGetValue(powerSource, CFSTR(kIOPSMaxCapacityKey));
    if (capacityRef == NULL) {
      continue;
    }

    int charge;
    int capacity;
    CFNumberGetValue(chargeRef, kCFNumberIntType, &charge);
    CFNumberGetValue(capacityRef, kCFNumberIntType, &capacity);
    assert(charge <= capacity);

    int currentBatteryChargePercent = (int)((100L * charge) / capacity);
    if (currentBatteryChargePercent < computerChargePercent) {
      computerChargePercent = currentBatteryChargePercent;
    }
  }

done:
  if (powerSourcesInfo != NULL) {
    CFRelease(powerSourcesInfo);
  }
  if (powerSourcesList != NULL) {
    CFRelease(powerSourcesList);
  }

  assert(computerChargePercent >= 0);
  assert(computerChargePercent <= 100);
  return computerChargePercent;
}

static void measureMemory(meter_sysload_t *load) {
  measureRam(load);

  load->memoryPressure = getMemoryPressure();
  if (load->memoryPressure >= 0) {
    if (load->memoryPressure > load->memoryPressureHighWatermark) {
      load->memoryPressureHighWatermark = load->memoryPressure;
    }
    if (load->memoryPressure < load->memoryPressureLowWatermark) {
      load->memoryPressureLowWatermark = load->memoryPressure;
    }
  }

  measureSwap(load);
}

static int getCpuCount(void) {
  int cpuCount;
  int mib[2U] = {CTL_HW, HW_NCPU};
  size_t sizeOfCpuCount = sizeof(cpuCount);
  int status = sysctl(mib, 2U, &cpuCount, &sizeOfCpuCount, NULL, 0U);
  assert(status == KERN_SUCCESS);
  return cpuCount;
}

static integer_t getCounterByCpu(processor_info_array_t cpuInfo, int cpuNumber,
                                 int counter) {
  return cpuInfo[CPU_STATE_MAX * cpuNumber + counter];
}

static void measureCpuLoad(meter_sysload_t *load) {
  // Measure CPU load, inspired by
  // http://stackoverflow.com/questions/6785069/get-cpu-percent-usage
  natural_t numCPUsU = 0U;
  processor_info_array_t cpuInfo;
  mach_msg_type_number_t numCpuInfo;
  kern_return_t err =
      host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &numCPUsU,
                          &cpuInfo, &numCpuInfo);
  assert(err == KERN_SUCCESS);

  for (int cpuIndex = 0; cpuIndex < load->nCpus; cpuIndex++) {
    integer_t loadValue =
        (getCounterByCpu(cpuInfo, cpuIndex, CPU_STATE_USER) +
         getCounterByCpu(cpuInfo, cpuIndex, CPU_STATE_SYSTEM));
    integer_t totalValue =
        (loadValue + getCounterByCpu(cpuInfo, cpuIndex, CPU_STATE_IDLE) +
         getCounterByCpu(cpuInfo, cpuIndex, CPU_STATE_NICE));
    accumulator_t *accumulator = load->cpuAccumulators[cpuIndex];
    accumulator_update(accumulator, loadValue, totalValue);
    load->cpuLoad[cpuIndex] = accumulator_get_percentage(accumulator);

    // Johan's MacBook can go up to 15% per core even when Johan thinks the
    // system is idle.  Censor load measurements to get a quiet meter on
    // idle system.
    if (load->cpuLoad[cpuIndex] < 15) {
      load->cpuLoad[cpuIndex] = 0;
    }
  }

  size_t cpuInfoSize = sizeof(integer_t) * numCpuInfo;
  vm_deallocate(mach_task_self(), (vm_address_t)cpuInfo, cpuInfoSize);
}

// This function is much inspired by record_device() and devstats() from
// http://opensource.apple.com/source/system_cmds/system_cmds-230/iostat.tproj/iostat.c
static void reportDrive(dynamic_accumulator_t *ioLoad,
                        io_registry_entry_t drive) {
  io_registry_entry_t parent;
  CFStringRef name;
  kern_return_t status;

  // get drive's parent
  status = IORegistryEntryGetParentEntry(drive, kIOServicePlane, &parent);
  if (status != KERN_SUCCESS) {
    return;
  }
  if (!IOObjectConformsTo(parent, "IOBlockStorageDriver")) {
    IOObjectRelease(parent);
    return;
  }

  // get drive properties
  CFMutableDictionaryRef driveProperties;
  status = IORegistryEntryCreateCFProperties(drive, &driveProperties,
                                             kCFAllocatorDefault, kNilOptions);
  if (status != KERN_SUCCESS) {
    return;
  }

  // get name from properties
  name =
      (CFStringRef)CFDictionaryGetValue(driveProperties, CFSTR(kIOBSDNameKey));
  char cname[100];
  CFStringGetCString(name, cname, sizeof(cname), CFStringGetSystemEncoding());
  CFRelease(driveProperties);

  // get parent properties
  CFMutableDictionaryRef parentProperties;
  status = IORegistryEntryCreateCFProperties(parent, &parentProperties,
                                             kCFAllocatorDefault, kNilOptions);
  IOObjectRelease(parent);
  if (status != KERN_SUCCESS) {
    CFRelease(driveProperties);
    return;
  }

  CFDictionaryRef statistics;
  statistics = (CFDictionaryRef)CFDictionaryGetValue(
      parentProperties, CFSTR(kIOBlockStorageDriverStatisticsKey));
  if (!statistics) {
    CFRelease(parentProperties);
    return;
  }

  u_int64_t bytesRead = 0;
  u_int64_t bytesWritten = 0;
  CFNumberRef number = (CFNumberRef)CFDictionaryGetValue(
      statistics, CFSTR(kIOBlockStorageDriverStatisticsBytesReadKey));
  if (number != NULL) {
    CFNumberGetValue(number, kCFNumberSInt64Type, &bytesRead);
  }

  number = (CFNumberRef)CFDictionaryGetValue(
      statistics, CFSTR(kIOBlockStorageDriverStatisticsBytesWrittenKey));
  if (number != NULL) {
    CFNumberGetValue(number, kCFNumberSInt64Type, &bytesWritten);
  }

  CFRelease(parentProperties);

  dynamic_accumulator_report(ioLoad, cname, bytesRead, bytesWritten);
}

// This function is much inspired by record_all_devices() from
// http://opensource.apple.com/source/system_cmds/system_cmds-230/iostat.tproj/iostat.c
static void measureIoLoad(meter_sysload_t *load) {
  dynamic_accumulator_t *ioLoad = (dynamic_accumulator_t *)load->user;
  dynamic_accumulator_startReporting(ioLoad);

  // For all devices..
  io_iterator_t drivelist;
  io_registry_entry_t drive;
  CFMutableDictionaryRef match;
  kern_return_t status;

  // Get an iterator for IOMedia objects.
  match = IOServiceMatching("IOMedia");
  CFDictionaryAddValue(match, CFSTR(kIOMediaWholeKey), kCFBooleanTrue);
  status = IOServiceGetMatchingServices(masterPort, match, &drivelist);
  assert(status == KERN_SUCCESS);

  // Scan all of the IOMedia objects, report each object that has a parent
  // IOBlockStorageDriver
  while ((drive = IOIteratorNext(drivelist))) {
    reportDrive(ioLoad, drive);

    IOObjectRelease(drive);
  }
  IOObjectRelease(drivelist);

  load->ioLoad = dynamic_accumulator_getLoadPercentage(ioLoad);
}

/* Initialize the load metering */
void meter_init(meter_sysload_t *load) {
#ifdef DEBUG
  dynamic_accumulator_selftest();
#endif

  load->user = dynamic_accumulator_create();

  initMemoryPressure(load);
  measureMemory(load);

  load->batteryChargePercent = getBatteryChargePercent();

  load->nCpus = getCpuCount();
  assert(load->nCpus > 0);
  load->cpuLoad = calloc(load->nCpus, sizeof(int));

  // Initialize the load histories and indices
  load->cpuAccumulators = calloc(load->nCpus, sizeof(accumulator_t *));
  for (int cpuNo = 0; cpuNo < load->nCpus; cpuNo++) {
    load->cpuAccumulators[cpuNo] =
        accumulator_create(LOADSAMPLES / MEASURE_LOAD_EVERY);
  }

  IOMasterPort(bootstrap_port, &masterPort);
  measureIoLoad(load);
}

/* Meter the system load */
void meter_getLoad(meter_sysload_t *load) {
  static int updateDelay = -1;
  if (updateDelay-- >= 0) {
    return;
  }
  updateDelay = MEASURE_LOAD_EVERY;

  measureMemory(load);
  measureCpuLoad(load);
  load->batteryChargePercent = getBatteryChargePercent();

  // Delay IO measurement even more; empirical studies show that this gives us
  // a lot better values.
  static int ioDelay = -1;
  if (ioDelay-- >= 0) {
    return;
  }
  // The magical number 2 comes from a trial-and-error session.  2 looks nice.
  ioDelay = 2;

  measureIoLoad(load);
}

/* Shut down load metering */
void meter_done(meter_sysload_t *load) {
  free(load->cpuLoad);
  load->cpuLoad = NULL;

  for (int i = 0; i < load->nCpus; i++) {
    accumulator_done(load->cpuAccumulators[i]);
  }
  free(load->cpuAccumulators);
  load->cpuAccumulators = NULL;

  dynamic_accumulator_destroy((dynamic_accumulator_t *)load->user);
}

mail_status_t mail_getMailStatus(void) { return NO_MAIL; }
