/*
 *  Bubbling Load Monitoring Applet
 *  - A GNOME panel applet that displays the CPU + memory load as a
 *    bubbling liquid.
 *  Copyright (C) 1999-2000 Johan Walles
 *  - d92-jwa@nada.kth.se
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

static int *cpuLoadIndex;
static int **cpuLoadHistory;
static int **cpuTotalLoadHistory;

/* Initialize the load metering */
void meter_init(int argc, char *argv[], meter_sysload_t *load)
{
  int cpuNo;
  glibtop_mem memory;
  glibtop_swap swap;
  
#if LIBGTOP_VERSION_CODE >= 1001005
  load->nCpus = glibtop_get_sysinfo()->ncpu;
#else
  load->nCpus = 1;
#endif  
  g_assert(load->nCpus > 0);

  // FIXME: Initialize the load histories and indices
  cpuLoadIndex = (int *)malloc(load->nCpus * sizeof(int));
  cpuLoadHistory = (int **)malloc(load->nCpus * sizeof(int *));
  cpuTotalLoadHistory = (int **)malloc(load->nCpus * sizeof(int *));

  for (cpuNo = 0; cpuNo < load->nCpus; cpuNo++)
  {
    cpuLoadHistory[cpuNo] = (int *)malloc(LOADSAMPLES * sizeof(int));
    cpuTotalLoadHistory[cpuNo] = (int *)malloc(LOADSAMPLES * sizeof(int));
  }
  
  // Initialize memory and swap sizes
  glibtop_get_mem(&memory);
  load->memorySize = memory.total;
  glibtop_get_swap(&swap);
  load->swapSize = swap.total;
}

/* Returns the current CPU load in percent */
static int getCpuLoad(int currentCpu, int nCpus)
{
  static glibtop_cpu cpu;
  int loadPercentage;
  u_int64_t my_user, my_system, my_total;
  u_int64_t load, total, oLoad, oTotal;
  int i;
  
  g_assert((currentCpu >= 0) && (currentCpu < nCpus));
  
  /* Find out the CPU load */
  if (currentCpu == 0)
  {
    glibtop_get_cpu (&cpu);
  }
  
  /* The following if() shouldn't be necessary, but according to the
     OpenBSD libgtop maintainer (nino@nforced.com) it is. */
  if (nCpus == 1)
    {
      my_user = cpu.user;
      my_system = cpu.sys;
      my_total = cpu.total;
    }
  else
    {
      my_user = cpu.xcpu_user[currentCpu];
      my_system = cpu.xcpu_sys[currentCpu];
      my_total = cpu.xcpu_total[currentCpu];
    }

  load = my_user + my_system;
  total = my_total;
  g_assert(total);

  /* "i" is an index into a load history */
  i = cpuLoadIndex[currentCpu];
  oLoad = cpuLoadHistory[currentCpu][i];
  oTotal = cpuTotalLoadHistory[currentCpu][i];

  cpuLoadHistory[currentCpu][i] = load;
  cpuTotalLoadHistory[currentCpu][i] = total;
  cpuLoadIndex[currentCpu] = (i + 1) % LOADSAMPLES;

  /*
    Because the load returned from libgtop is a value accumulated
    over time, and not the current load, the current load percentage
    is calculated as the extra amount of work that has been performed
    since the last sample.
  */
  if (oTotal == 0)  /* oTotal == 0 means that this is the first time
		       we get here */
  {
    loadPercentage = 0;
  }
  else
  {
    loadPercentage = (100 * (load - oLoad)) / (total - oTotal);
  }

  return loadPercentage;
}

/* Meter the system load */
void meter_getLoad(meter_sysload_t *load)
{
  static glibtop_mem memory;
  static glibtop_swap swap;

  static int memUpdateDelay = 0;

  int cpuNo;

  if (memUpdateDelay <= 0)
  {
    glibtop_get_mem (&memory);
    glibtop_get_swap (&swap);

    memUpdateDelay = MEMUPDATEDELAY;
  }
  memUpdateDelay--;
  
  // Fill in memory and swap usage
  load->memoryUsed =
    memory.total - (memory.free + memory.cached + memory.buffer);
  load->swapUsed = swap.used;
  
  for (cpuNo = 0; cpuNo < load->nCpus; cpuNo++) {
    load->cpuLoad[cpuNo] = getCpuLoad(cpuNo, load->nCpus);
  }
}

/* Shut down load metering */
void meter_done()
{
  // FIXME: We could free the load history stuff here
}
