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

/* Initialize the load metering */
void meter_init(meter_sysload_t *load) {
    load->memoryUsed = 100;
    load->memorySize = 200;
    load->swapUsed = 0;
    load->swapSize = 0;
    load->nCpus = 1;
    load->cpuLoad = calloc(1, sizeof(int));
    load->ioLoad = 0;
}

/* Meter the system load */
void meter_getLoad(meter_sysload_t *load) {
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
