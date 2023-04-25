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

#ifndef DYNAMIC_ACCUMULATOR_H
#define DYNAMIC_ACCUMULATOR_H

#include <inttypes.h>
#include <stdbool.h>

/* See dynamic_accumulator_t below.
 */
struct device_t;
typedef struct device_t device_t;
struct device_t {
  char *name;

  u_int64_t lastTimestampMs;

  u_int64_t lastReadBytes;
  u_int64_t lastWrittenBytes;

  double maxReadsPerSecond;
  double maxWritesPerSecond;

  double lastReadsPerSecond;
  double lastWritesPerSecond;

  // Was this device reported during the last reporting run?
  bool reported;

  device_t *next;
};

/* Holds info about a dynamic accumulator.  A dynamic accumulator can
 * be used to emulate the Linux-specific iowait number on non-Linux
 * systems.
 *
 * Example usage:
 * dynamic_accumulator_report(da, "hda", 1000 [read], 0 [written]);
 * dynamic_accumulator_report(da, "hdb", 4711 [read], 99999 [written]);
 * dynamic_accumulator_getLoadPercentage(da);
 *
 * The report() function reports a device name and the number of bytes
 * read+written since system boot.  What the accumulator does is to
 * keep track of the highest amount of bytes per second for each
 * device.
 *
 * The load percentage for any device is how many percent of the
 * maximum bandwidth was transmitted between the last two reports.
 *
 * The value reported by getLoadPercentage() is the load percentage
 * for the most heavily loaded device between the last two reports.
 */
typedef struct {
  device_t *devices;

  // Are we currently reporting devices?  Reporting starts with
  // dynamic_accumilator_startReporting() and ends with
  // dynamic_accumulator_getLoadPercentage().
  bool reporting;
} dynamic_accumulator_t;

/* Create a new dynamic accumulator */
dynamic_accumulator_t *dynamic_accumulator_create(void);

/* Call before starting a reporting sequence, needed to be able to keep track of
 * when devices disappear.
 */
void dynamic_accumulator_startReporting(
    dynamic_accumulator_t *dynamic_accumulator);

/* Update a dynamic accumulator with current values. */
void dynamic_accumulator_report(dynamic_accumulator_t *dynamic_accumulator,
                                const char *deviceName, u_int64_t bytesRead,
                                u_int64_t bytesWritten);

/* Fetch the current load percentage value (0-100). */
int dynamic_accumulator_getLoadPercentage(
    dynamic_accumulator_t *dynamic_accumulator);

/* Free a dynamic accumulator struct. */
void dynamic_accumulator_destroy(dynamic_accumulator_t *dynamic_accumulator);

/* Run self tests, assert()s will fail on failures */
void dynamic_accumulator_selftest(void);

#endif
