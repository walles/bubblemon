//
//  osx-meter.c
//  bubblemon
//
//  Created by Johan Walles on 2012-04-29.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "meter.h"
#include "mail.h"

#include <stdlib.h>
#include <sys/sysctl.h>
#include <mach/host_info.h>
#include <mach/mach_host.h>
#include <mach/task_info.h>
#include <mach/task.h>
#include <assert.h>

static void measureSwap(meter_sysload_t *load) {
    int vmmib[2] = {CTL_VM, VM_SWAPUSAGE};
    struct xsw_usage swapused; /* defined in sysctl.h */
    size_t swlen = sizeof(swapused);
    assert(sysctl(vmmib, 2, &swapused, &swlen, NULL, 0) >= 0);
    
    load->swapUsed = swapused.xsu_used ;
    load->swapSize = swapused.xsu_total;
}

int getPageSize() {
    int mib[6]; 
    mib[0] = CTL_HW;
    mib[1] = HW_PAGESIZE;
    
    int pageSize;
    size_t length;
    length = sizeof (pageSize);
    assert(sysctl(mib, 2, &pageSize, &length, NULL, 0) >= 0);
    
    return pageSize;
}

static void measureRam(meter_sysload_t *load) {
    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    
    vm_statistics_data_t vmstat;
    assert(host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vmstat, &count) == KERN_SUCCESS);
    
    natural_t total = vmstat.wire_count + vmstat.active_count + vmstat.inactive_count + vmstat.free_count;
    int pageSize = getPageSize();
    load->memorySize = total * pageSize;
    load->memoryUsed = (vmstat.wire_count + vmstat.active_count) * pageSize;
}

static void measureMemory(meter_sysload_t *load) {
    measureRam(load);
    measureSwap(load);
}

/* Initialize the load metering */
void meter_init(meter_sysload_t *load) {
    measureMemory(load);
    load->nCpus = 1;
    load->cpuLoad = calloc(1, sizeof(int));
    load->ioLoad = 0;
}

/* Meter the system load */
void meter_getLoad(meter_sysload_t *load) {
    measureMemory(load);
    load->cpuLoad[0] = 30;
}

/* Shut down load metering */
void meter_done(meter_sysload_t *load) {
    free(load->cpuLoad);
    load->cpuLoad = NULL;
}

mail_status_t mail_getMailStatus(void) {
    return NO_MAIL;
}
