/*
 *  Bubbling Load Monitoring Applet
 *  - A GNOME panel applet that displays the CPU + memory load as a
 *    bubbling liquid.
 *  Copyright (C) 1999-2004, 2009 Johan Walles
 *  - johan.walles@gmail.com
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

#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>
#include <glibtop/sysinfo.h>

#include <config.h>

#include "meter.h"

// How large is the load history?  Large values gives good precision
// but over a long time.  Small values gives worse precision but more
// current values.
#define LOADSAMPLES 16

// How often is the memory information updated?  Fetching memory
// information from libgtop is very costly on some systems, so setting
// this value to zero is not recommended.
#define MEMUPDATEDELAY 25

// How often is the CPU and IO load information updated?  Fetching
// this information adds a measurable kernel load, so this is a way to
// keep that down.
#define LOADUPDATEDELAY 3

/* Initialize the load metering */
void meter_init(meter_sysload_t *load)
{
  int cpuNo;
  glibtop_mem memory;
  glibtop_swap swap;

  load->nCpus = glibtop_get_sysinfo()->ncpu;
  g_assert(load->nCpus > 0);
  load->cpuLoad = (int *)calloc(load->nCpus, sizeof(int));
  g_assert(load->cpuLoad != NULL);

  // Initialize the load histories and indices
  load->cpuAccumulators = calloc(load->nCpus, sizeof(accumulator_t));
  g_assert(load->cpuAccumulators != NULL);
  for (cpuNo = 0; cpuNo < load->nCpus; cpuNo++) {
    load->cpuAccumulators[cpuNo] = accumulator_create(LOADSAMPLES);
  }

  load->ioAccumulators = calloc(load->nCpus, sizeof(accumulator_t));
  g_assert(load->ioAccumulators != NULL);
  for (cpuNo = 0; cpuNo < load->nCpus; cpuNo++) {
    load->ioAccumulators[cpuNo] = accumulator_create(LOADSAMPLES);
  }

  // Initialize memory and swap sizes
  glibtop_get_mem(&memory);
  load->memorySize = memory.total;
  glibtop_get_swap(&swap);
  load->swapSize = swap.total;
}

/* Meter the system load */
void meter_getLoad(meter_sysload_t *meter)
{
  glibtop_cpu cpu;
  glibtop_mem memory;
  glibtop_swap swap;

  int cpuIndex;
  u_int64_t ioSum;

  static int memUpdateDelay = 0;

  if (memUpdateDelay <= 0)
  {
    // Fill in memory and swap usage
    glibtop_get_mem (&memory);
    glibtop_get_swap (&swap);

    meter->memoryUsed =
      memory.total - (memory.free + memory.cached + memory.buffer);
    meter->swapUsed = swap.used;

    memUpdateDelay = MEMUPDATEDELAY;
  }
  memUpdateDelay--;

  static int loadUpdateDelay = 0;
  if (loadUpdateDelay <= 0)
  {
     // Fill in cpu and IO usage
     glibtop_get_cpu(&cpu);
     ioSum = 0;
     for (cpuIndex = 0; cpuIndex < meter->nCpus; cpuIndex++) {
        u_int64_t myUser, mySystem, myIowait, myLoad, myTotal;
        
        // The following if() shouldn't be necessary, but according to the
        // OpenBSD libgtop maintainer (nino@nforced.com) it is.
        if (meter->nCpus == 1) {
           myUser = cpu.user;
           mySystem = cpu.sys;
           myIowait = cpu.iowait;
           myTotal = cpu.total;
        } else {
           myUser = cpu.xcpu_user[cpuIndex];
           mySystem = cpu.xcpu_sys[cpuIndex];
           myIowait = cpu.xcpu_iowait[cpuIndex];
           myTotal = cpu.xcpu_total[cpuIndex];
        }
        myLoad = myUser + mySystem;
        
        accumulator_update(meter->cpuAccumulators[cpuIndex], myLoad, myTotal);
        meter->cpuLoad[cpuIndex] = accumulator_get_percentage(meter->cpuAccumulators[cpuIndex]);
        
        accumulator_update(meter->ioAccumulators[cpuIndex], myIowait, myTotal);
        ioSum += accumulator_get_percentage(meter->ioAccumulators[cpuIndex]);
     }
     if (ioSum > 100) {
        ioSum = 100;
     }
     meter->ioLoad = ioSum;

     loadUpdateDelay = LOADUPDATEDELAY;
  }
  loadUpdateDelay--;
}

/* Shut down load metering */
void meter_done(meter_sysload_t *meter)
{
  int cpuIndex;

  for (cpuIndex = 0; cpuIndex < meter->nCpus; cpuIndex++) {
    accumulator_done(meter->cpuAccumulators[cpuIndex]);
    meter->cpuAccumulators[cpuIndex] = NULL;

    accumulator_done(meter->ioAccumulators[cpuIndex]);
    meter->ioAccumulators[cpuIndex] = NULL;
  }
  free(meter->cpuAccumulators);
  free(meter->ioAccumulators);
  free(meter->cpuLoad);
}
