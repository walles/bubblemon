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

#ifndef ACKUMULATOR_H
#define ACKUMULATOR_H

#include <sys/types.h>

/* Holds info about an ackumulator.  An ackumulator measures something
 * using two values; one timer counter and one events counter.  Given
 * such a value, the functions in this file can be used to find out
 * the current load in percent. */
typedef struct
{
  /* History size.  Larger sizes gives better values, but will also
   * make the computed value lag longer after reality. */
  unsigned int historySize;

  /* The load history */
  u_int64_t *loadHistory;

  /* The total possible load history */
  u_int64_t *totalHistory;

  /* Our current position in the history */
  int loadIndex;

  /* Last computed value */
  int percentage;
} ackumulator_t;

/* Create a new ackumulator */
ackumulator_t *ackumulator_create(unsigned int historySize);

/* Update an ackumulator with current values. */
void ackumulator_update(ackumulator_t *ackumulator,
			u_int64_t loadValue,
			u_int64_t totalValue);

/* Fetch the current load percentage value (0-100). */
int ackumulator_get_percentage(ackumulator_t *ackumulator);

/* Free an ackumulator struct. */
void ackumulator_done(ackumulator_t *ackumulator);

#endif
