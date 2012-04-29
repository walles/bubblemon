//
//  osx-meter.c
//  bubblemon
//
//  Created by Johan Walles on 2012-04-29.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "meter.h"
#include "mail.h"

/* Initialize the load metering */
void meter_init(meter_sysload_t *load) {
}

/* Meter the system load */
void meter_getLoad(meter_sysload_t *load) {
}

/* Shut down load metering */
void meter_done(meter_sysload_t *load) {
}

mail_status_t mail_getMailStatus(void) {
    return NO_MAIL;
}
