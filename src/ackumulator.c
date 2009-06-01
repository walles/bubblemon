/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 2009 Johan Walles - johan.walles@gmail.com
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

/* An ackumulator measures something using two values; one timer
 * counter and one events counter.  Given those values, the functions
 * in this file can be used to find out the current load in
 * percent. */

#include <assert.h>
#include <stdlib.h>

#include "ackumulator.h"

/* Create a new ackumulator */
ackumulator_t *ackumulator_create(unsigned int historySize) {
  ackumulator_t *ackumulator = calloc(1, sizeof(ackumulator_t));
  assert(ackumulator != NULL);

  ackumulator->historySize = historySize;

  ackumulator->loadHistory = calloc(historySize, sizeof(u_int64_t));
  assert(ackumulator->loadHistory != NULL);

  ackumulator->totalHistory = calloc(historySize, sizeof(u_int64_t));
  assert(ackumulator->totalHistory != NULL);

  return ackumulator;
}

/* Update an ackumulator with current values. */
void ackumulator_update(ackumulator_t *ackumulator,
			u_int64_t load, u_int64_t total)
{
  int i;
  u_int64_t lastLoad;
  u_int64_t lastTotal;

  /* "i" is an index into a load history */
  i = ackumulator->loadIndex;
  lastLoad = ackumulator->loadHistory[i];
  lastTotal = ackumulator->totalHistory[i];

  if (total - lastTotal == 0) {
    // No ticks have elapsed, load cannot be measured (Debian bug #220255)
    return;
  }

  ackumulator->loadHistory[i] = load;
  ackumulator->totalHistory[i] = total;
  ackumulator->loadIndex = (i + 1) % ackumulator->historySize;

  // Calculate load as the extra amount of work that has been
  // performed since the last sample.
  if (lastTotal == 0) {
    // This is the first time we get here
    ackumulator->percentage = 0;
  } else {
    ackumulator->percentage = (100 * (load - lastLoad)) / (total - lastTotal);
  }

  if (ackumulator->percentage > 100) {
    // Clip percentage at 100%
    ackumulator->percentage = 100;
  }

  // We should never get < 0% load
  assert(ackumulator->percentage >= 0);
}

/* Fetch the current load percentage value (0-100). */
int ackumulator_get_percentage(ackumulator_t *ackumulator) {
  return ackumulator->percentage;
}

/* Free an ackumulator struct. */
void ackumulator_done(ackumulator_t *ackumulator) {
  free(ackumulator->loadHistory);
  ackumulator->loadHistory = NULL;

  free(ackumulator->totalHistory);
  ackumulator->totalHistory = NULL;

  free(ackumulator);
}
