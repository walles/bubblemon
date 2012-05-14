/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2004, 2008, 2009, 2010, 2012 Johan Walles - johan.walles@gmail.com
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

#ifndef XFCE4_UI_H
#define XFCE4_UI_H

#include "bubblemon.h"

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <libxfce4panel/xfce-panel-plugin.h>

#define FRAMERATE 25

typedef struct
{
  XfcePanelPlugin *plugin;
  GtkWidget *frame_widget;
  GtkWidget *draw_area;
  GtkWidget *box;
  GdkPixmap *pix;

  guchar *rgb_buffer;

  int width;
  int height;

  guint refresh_timeout_id;
  guint tooltip_timeout_id;

  bubblemon_t *bubblemon;
} Bubblemon;

static void bubblemon_construct (XfcePanelPlugin *plugin);

#endif
