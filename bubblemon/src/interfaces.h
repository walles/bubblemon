/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2007 Johan Walles - johan.walles@gmail.com
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

/*
 * This is the header file for listing available network interfaces.
 */
#ifndef INTERFACES_H_
#define INTERFACES_H_

/*
 * Return an array of names of potentially active interfaces.  The last name must be
 * NULL.  When you're done with these results, call interface_freecandidates().
 */
char **interfaces_getcandidates(void);

/* Call this after accessing the result from interface_getcandidates(). */
void interfaces_freecandidates(char** candidates);

#endif // INTERFACES_H_
