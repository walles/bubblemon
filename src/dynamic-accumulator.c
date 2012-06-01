/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 2012 Johan Walles - johan.walles@gmail.com
 *  http://www.nongnu.org/bubblemon/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

#include "dynamic-accumulator.h"

static u_int64_t timeMs;

static void setTimeMs(u_int64_t ms) {
  timeMs = ms;
}

static void setTime(void) {
  struct timeval now;
  gettimeofday(&now, NULL);
  timeMs = 1000L * now.tv_sec;
  timeMs += now.tv_usec / 1000;
}

dynamic_accumulator_t *dynamic_accumulator_create(void) {
  return (dynamic_accumulator_t*)calloc(1, sizeof(dynamic_accumulator_t));
}

void dynamic_accumulator_startReporting(dynamic_accumulator_t *dynamic_accumulator) {
  assert("Must do getLoadPercentage() before reporting again" &&
         !dynamic_accumulator->reporting);
  
  dynamic_accumulator->reporting = true;
  for (device_t *device = dynamic_accumulator->devices; device != NULL; device = device->next) {
    device->reported = false;
  }
}

static device_t *findOrCreateDevice(dynamic_accumulator_t *dynamic_accumulator,
                                    const char *name)
{
  for (device_t *device = dynamic_accumulator->devices; device != NULL; device = device->next) {
    if (strcmp(name, device->name) == 0) {
      return device;
    }
  }
  
  // Not found, create a new one
  device_t *device = (device_t*)calloc(1, sizeof(device_t));
  device->name = strdup(name);
  device->next = dynamic_accumulator->devices;
  dynamic_accumulator->devices = device;
  return device;
}

static void report(dynamic_accumulator_t *dynamic_accumulator,
                   const char *deviceName,
                   u_int64_t bytesRead,
                   u_int64_t bytesWritten)
{
  assert("Must call startReporting() before reporting devices" &&
         dynamic_accumulator->reporting);
  
  device_t *device = findOrCreateDevice(dynamic_accumulator, deviceName);
  
  assert("Must report each device only once" &&
         !device->reported);
  
  double secondsThisInterval = (timeMs - device->lastTimestampMs) / 1000.0;
  
  double bytesReadThisInterval = bytesRead - device->lastReadBytes;
  double bytesWrittenThisInterval = bytesWritten - device->lastWrittenBytes;
  
  if (device->lastTimestampMs > 0) {
    device->lastReadsPerSecond = bytesReadThisInterval / secondsThisInterval;
    device->lastWritesPerSecond = bytesWrittenThisInterval / secondsThisInterval;
    
    if (device->lastReadsPerSecond > device->maxReadsPerSecond) {
      device->maxReadsPerSecond = device->lastReadsPerSecond;
    }
    if (device->lastWritesPerSecond > device->maxWritesPerSecond) {
      device->maxWritesPerSecond = device->lastWritesPerSecond;
    }
  }
  
  device->lastTimestampMs = timeMs;
  device->lastReadBytes = bytesRead;
  device->lastWrittenBytes = bytesWritten;
  device->reported = true;
}

void dynamic_accumulator_report(dynamic_accumulator_t *dynamic_accumulator,
                                const char *device,
				u_int64_t bytesRead,
				u_int64_t bytesWritten)
{
  setTime();
  report(dynamic_accumulator, device, bytesRead, bytesWritten);
}

static void deleteUnreportedDevices(dynamic_accumulator_t *dynamic_accumulator) {
  device_t **devicePointer = &(dynamic_accumulator->devices);
  while (*devicePointer != NULL) {
    device_t *currentDevice = *devicePointer;
    if (currentDevice->reported) {
      devicePointer = &(currentDevice->next);
      continue;
    }
    
    // Delete unreported device
    *devicePointer = currentDevice->next;
    free(currentDevice->name);
    free(currentDevice);
  }
}

int dynamic_accumulator_getLoadPercentage(dynamic_accumulator_t *dynamic_accumulator) {
  if (dynamic_accumulator->reporting) {
    deleteUnreportedDevices(dynamic_accumulator);
  }
  dynamic_accumulator->reporting = false;
  
  double maxLoadFraction = 0.0;
  for (device_t *device = dynamic_accumulator->devices; device != NULL; device = device->next) {
    double readLoadFraction = 0.0;
    if (device->maxReadsPerSecond > 0.0) {
      readLoadFraction = device->lastReadsPerSecond / device->maxReadsPerSecond;
    }
    if (readLoadFraction > maxLoadFraction) {
      maxLoadFraction = readLoadFraction;
    }

    double writeLoadFraction = 0.0;
    if (device->maxWritesPerSecond > 0.0) {
      writeLoadFraction = device->lastWritesPerSecond / device->maxWritesPerSecond;
    }
    if (writeLoadFraction > maxLoadFraction) {
      maxLoadFraction = writeLoadFraction;
    }
  }
  
  return round(maxLoadFraction * 100.0);
}

void dynamic_accumulator_destroy(dynamic_accumulator_t *dynamic_accumulator) {
  device_t *nextDevice;
  for (device_t *device = dynamic_accumulator->devices; device != NULL; device = nextDevice) {
    nextDevice = device->next;
    free(device->name);
    free(device);
  }
  free(dynamic_accumulator);
}

static void testCreateDestroy() {
  dynamic_accumulator_t *testMe = dynamic_accumulator_create();
  assert(testMe != NULL);
  
  dynamic_accumulator_startReporting(testMe);
  dynamic_accumulator_report(testMe, "sda", 42, 42);
  dynamic_accumulator_getLoadPercentage(testMe);
  
  dynamic_accumulator_destroy(testMe);
}

static void testNoData() {
  dynamic_accumulator_t *testMe = dynamic_accumulator_create();
  assert(dynamic_accumulator_getLoadPercentage(testMe) == 0);
  dynamic_accumulator_destroy(testMe);
}

static void testSingleDevice() {
  dynamic_accumulator_t *testMe = dynamic_accumulator_create();

  setTimeMs(5100);
  dynamic_accumulator_startReporting(testMe);
  report(testMe, "sda", 100, 100);
  assert(dynamic_accumulator_getLoadPercentage(testMe) == 0);
  
  setTimeMs(5200);
  dynamic_accumulator_startReporting(testMe);
  report(testMe, "sda", 200, 200);
  assert(dynamic_accumulator_getLoadPercentage(testMe) == 100);

  setTimeMs(5300);
  dynamic_accumulator_startReporting(testMe);
  report(testMe, "sda", 250, 250);
  assert(dynamic_accumulator_getLoadPercentage(testMe) == 50);
  
  setTimeMs(5400);
  dynamic_accumulator_startReporting(testMe);
  report(testMe, "sda", 450, 450);
  assert(dynamic_accumulator_getLoadPercentage(testMe) == 100);
  
  setTimeMs(5500);
  dynamic_accumulator_startReporting(testMe);
  report(testMe, "sda", 550, 550);
  assert(dynamic_accumulator_getLoadPercentage(testMe) == 50);
  
  dynamic_accumulator_destroy(testMe);
}

static void testGcFirstDeviceOfTwo() {
  dynamic_accumulator_t *testMe = dynamic_accumulator_create();
  
  // Add two devices
  dynamic_accumulator_startReporting(testMe);
  setTimeMs(100);
  report(testMe, "first", 100, 100);
  report(testMe, "second", 100, 100);
  assert(dynamic_accumulator_getLoadPercentage(testMe) == 0);

  // Load them a bit
  dynamic_accumulator_startReporting(testMe);
  setTimeMs(200);
  report(testMe, "first", 200, 200);
  report(testMe, "second", 200, 200);
  assert(dynamic_accumulator_getLoadPercentage(testMe) == 100);
  
  // Stop reporting about one of them
  dynamic_accumulator_startReporting(testMe);
  setTimeMs(300);
  report(testMe, "second", 300, 300);
  assert(dynamic_accumulator_getLoadPercentage(testMe) == 100);
  assert(testMe->devices != NULL);
  assert(testMe->devices->next == NULL);
  assert(strcmp("second", testMe->devices->name) == 0);
  
  dynamic_accumulator_destroy(testMe);
}

static void testGcSecondDeviceOfTwo() {
  dynamic_accumulator_t *testMe = dynamic_accumulator_create();
  
  // Add two devices
  dynamic_accumulator_startReporting(testMe);
  setTimeMs(100);
  report(testMe, "first", 100, 100);
  report(testMe, "second", 100, 100);
  assert(dynamic_accumulator_getLoadPercentage(testMe) == 0);
  
  // Load them a bit
  dynamic_accumulator_startReporting(testMe);
  setTimeMs(200);
  report(testMe, "first", 200, 200);
  report(testMe, "second", 200, 200);
  assert(dynamic_accumulator_getLoadPercentage(testMe) == 100);
  
  // Stop reporting about one of them
  dynamic_accumulator_startReporting(testMe);
  setTimeMs(300);
  report(testMe, "first", 300, 300);
  assert(dynamic_accumulator_getLoadPercentage(testMe) == 100);
  assert(testMe->devices != NULL);
  assert(testMe->devices->next == NULL);
  assert(strcmp("first", testMe->devices->name) == 0);
  
  dynamic_accumulator_destroy(testMe);
}

void dynamic_accumulator_selftest(void) {
  testCreateDestroy();
  testNoData();
  testSingleDevice();
  testGcFirstDeviceOfTwo();
  testGcSecondDeviceOfTwo();
  // FIXME: testCountersMovingBackwards();
  // FIXME: testTimeMovingBackwards();
}
