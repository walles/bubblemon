/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2004, 2008, 2009, 2010 Johan Walles - johan.walles@gmail.com
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

#ifndef GNOME2_UI_H
#define GNOME2_UI_H

#include <panel-applet.h>
#include <sys/types.h>

#include "bubblemon.h"

#define FRAMERATE 25

typedef struct
{
  GtkWidget *applet;
  GtkWidget *drawingArea;
  GdkPixmap *pix;

  guchar *rgb_buffer;

  int width;
  int height;

  guint refresh_timeout_id;
  guint tooltip_timeout_id;

  // Reference to our load-metering, bubble-drawing, tool-tipping
  // bubblemon thingy.
  bubblemon_t *bubblemon;
} BubblemonApplet;

#endif
