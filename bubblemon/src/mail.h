/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2004 Johan Walles - walles@mailblocks.com
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

#ifndef MAIL_H
#define MAIL_H

#include "bubblemon.h"

/* Does the user have unread mail?
 *
 * Note that this function will be called once per on-screen frame, so
 * in case it always takes lots of time to evaluate the bubblemon will
 * get *slow*. */
extern int mail_hasUnreadMail(void);

#endif
