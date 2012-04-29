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

/* An accumulator measures something using two values; one timer
 * counter and one events counter.  Given those values, the functions
 * in this file can be used to find out the current load in
 * percent. */

#include <assert.h>
#include <stdlib.h>

#include "accumulator.h"

/* Create a new accumulator */
accumulator_t *accumulator_create(unsigned int historySize) {
  accumulator_t *accumulator = calloc(1, sizeof(accumulator_t));
  assert(accumulator != NULL);

  accumulator->historySize = historySize;

  accumulator->loadHistory = calloc(historySize, sizeof(u_int64_t));
  assert(accumulator->loadHistory != NULL);

  accumulator->totalHistory = calloc(historySize, sizeof(u_int64_t));
  assert(accumulator->totalHistory != NULL);

  return accumulator;
}

/* Update an accumulator with current values. */
void accumulator_update(accumulator_t *accumulator,
			u_int64_t load, u_int64_t total)
{
  int i;
  u_int64_t lastLoad;
  u_int64_t lastTotal;

  /* "i" is an index into a load history */
  i = accumulator->loadIndex;
  lastLoad = accumulator->loadHistory[i];
  lastTotal = accumulator->totalHistory[i];

  if (total - lastTotal == 0) {
    // No ticks have elapsed, load cannot be measured (Debian bug #220255)
    return;
  }

  accumulator->loadHistory[i] = load;
  accumulator->totalHistory[i] = total;
  accumulator->loadIndex = (i + 1) % accumulator->historySize;

  // Calculate load as the extra amount of work that has been
  // performed since the last sample.
  if (lastTotal == 0) {
    // This is the first time we get here
    accumulator->percentage = 0;
  } else {
    accumulator->percentage = (100 * (load - lastLoad)) / (total - lastTotal);
  }

  if (accumulator->percentage > 100) {
    // Clip percentage at 100%
    accumulator->percentage = 100;
  }

  // We should never get < 0% load
  assert(accumulator->percentage >= 0);
}

/* Fetch the current load percentage value (0-100). */
int accumulator_get_percentage(accumulator_t *accumulator) {
  return accumulator->percentage;
}

/* Free an accumulator struct. */
void accumulator_done(accumulator_t *accumulator) {
  free(accumulator->loadHistory);
  accumulator->loadHistory = NULL;

  free(accumulator->totalHistory);
  accumulator->totalHistory = NULL;

  free(accumulator);
}
