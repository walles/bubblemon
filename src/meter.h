/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2004, 2009 Johan Walles - johan.walles@gmail.com
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

#ifndef METER_H
#define METER_H

#include "accumulator.h"

/* The system load */
typedef struct
{
  u_int64_t memoryUsed;
  u_int64_t memorySize;

  u_int64_t swapUsed;
  u_int64_t swapSize;

  /* How many CPUs are in the system */
  int nCpus;

  /* A pointer to an array containing the loads of each of the
     system's CPUs in percent (0-100).  Index 0 is the average CPU
     load for the whole system, index 1 and up are the loads on the
     individual CPUs. */
  int *cpuLoad;

  /* How much of the system's IO bandwidth is in use */
  int ioLoad;

  /* How much battery charge is left.
     0=empty
     100=full

     If this system has no battery, 100 will be returned. */
  int batteryCharge;

  // FIXME: Having the below fields as part of the official API might
  // not be the best idea ever /Johan-2009feb22
  accumulator_t **cpuAccumulators;
  accumulator_t **ioAccumulators;
  
  // Arbitrary platform specific data goes here
  void *user;
} meter_sysload_t;

/* Initialize the load metering */
extern void meter_init(meter_sysload_t *);

/* Meter the system load */
extern void meter_getLoad(meter_sysload_t *);

/* Shut down load metering */
extern void meter_done(meter_sysload_t *);

#endif
