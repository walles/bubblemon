/*
 *  Walles Bubbling Load Monitoring Applet
 *  - A GNOME panel applet that displays the CPU + memory load as a
 *    bubbling liquid.
 *  Copyright (C) 1999 Johan Walles
 *  - d92-jwa@nada.kth.se
 *  Copyright (C) 1999 Merlin Hughes
 *  - merlin@merlin.org
 *  - http://nitric.com/freeware/
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

#include <applet-widget.h>

#ifndef _PROPERTIES_H
#define _PROPERTIES_H

void bubblemon_properties_window(AppletWidget * applet, gpointer data);
void col_value_changed_cb( GtkObject * ignored, guint arg1, guint arg2,
			   guint arg3, guint arg4, gpointer data );
void adj_value_changed_cb( GtkAdjustment * ignored, gpointer data );

#endif /* _PROPERTIES_H */
