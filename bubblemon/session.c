/*
 *  Bubbling Load Monitoring Applet
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

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gnome.h>
#include <gdk/gdkx.h>

#include <applet-widget.h>

#include "bubblemon.h"
#include "session.h"

void
bubblemon_session_load(gchar * cfgpath, BubbleMonData * mc)
{

  /* We specify that we want the properties for this applet ... */
  gnome_config_push_prefix (cfgpath);

  /* Global configurable parameters */
  mc->breadth = gnome_config_get_int_with_default
    ("bubblemon/width=" BUBBLEMON_DEFAULT_BREADTH, NULL);

  mc->depth = gnome_config_get_int_with_default
    ("bubblemon/height=" BUBBLEMON_DEFAULT_DEPTH, NULL);

  mc->update = gnome_config_get_int_with_default
    ("bubblemon/update=" BUBBLEMON_DEFAULT_UPDATE_RATE, NULL);

  mc->samples = gnome_config_get_int_with_default
    ("bubblemon/samples=" BUBBLEMON_DEFAULT_LOAD_SAMPLES, NULL);

  strncpy(mc->background_s,
	  gnome_config_get_string_with_default
	  ("bubblemon/background=" BUBBLEMON_DEFAULT_BACKGROUND, NULL),
	  24 /* sizeof(mc->background_s) */);

  gnome_config_pop_prefix ();
} /* bubblemon_session_load */

int
bubblemon_session_save(GtkWidget * w,
		     const char * privcfgpath,
		     const char * globcfgpath,
		     gpointer data)
{
  BubbleMonData * mc = data;

  gnome_config_push_prefix (privcfgpath);

  /* Global configurable parameters */
  gnome_config_set_int ("bubblemon/width", mc->breadth);
  gnome_config_set_int ("bubblemon/height", mc->depth);
  gnome_config_set_int ("bubblemon/update", mc->update);
  gnome_config_set_int ("bubblemon/samples", mc->samples);
  gnome_config_set_string ("bubblemon/background", mc->background_s);

  gnome_config_pop_prefix ();

  gnome_config_sync ();
  gnome_config_drop_all ();

  return FALSE;
} /* bubblemon_session_save */

void
bubblemon_session_defaults(BubbleMonData * mc)
{
  /* Global configurable parameters */
  mc->breadth = atoi (BUBBLEMON_DEFAULT_BREADTH);
  mc->depth = atoi (BUBBLEMON_DEFAULT_DEPTH);
  mc->update = atoi (BUBBLEMON_DEFAULT_UPDATE_RATE);
  mc->samples = atoi (BUBBLEMON_DEFAULT_LOAD_SAMPLES);
  strncpy(mc->background_s, BUBBLEMON_DEFAULT_BACKGROUND, 24 /* sizeof(mc->background_s) */);
} /* bubblemon_session_defaults */
