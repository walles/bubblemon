/*
 *  Bubbling Load Monitoring Applet
 *  - A GNOME panel applet that displays the CPU + memory load as a
 *    bubbling liquid.
 *  Copyright (C) 1999 Johan Walles
 *  - d92-jwa@nada.kth.se
 *  Copyright (C) 1999 Merlin Hughes
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
bubblemon_session_load(gchar * cfgpath, BubbleMonData * bm)
{

  /* We specify that we want the properties for this applet ... */
  gnome_config_push_prefix (cfgpath);

  /* Global configurable parameters */
  bm->update = gnome_config_get_int_with_default
    ("bubblemon/update=" BUBBLEMON_DEFAULT_UPDATE_RATE, NULL);

  bm->samples = gnome_config_get_int_with_default
    ("bubblemon/samples=" BUBBLEMON_DEFAULT_LOAD_SAMPLES, NULL);

  sscanf (BUBBLEMON_DEFAULT_AIR_NOSWAP, "#%x", &(bm->air_noswap));
  sscanf (BUBBLEMON_DEFAULT_LIQUID_NOSWAP, "#%x", &(bm->liquid_noswap));
  sscanf (BUBBLEMON_DEFAULT_AIR_MAXSWAP, "#%x", &(bm->air_maxswap));
  sscanf (BUBBLEMON_DEFAULT_LIQUID_MAXSWAP, "#%x", &(bm->liquid_maxswap));

  gnome_config_pop_prefix ();
} /* bubblemon_session_load */

int
bubblemon_session_save(GtkWidget * w,
		     const char * privcfgpath,
		     const char * globcfgpath,
		     gpointer data)
{
  BubbleMonData * bm = data;

  gnome_config_push_prefix (privcfgpath);

  /* Global configurable parameters */
  gnome_config_set_int ("bubblemon/update", bm->update);
  gnome_config_set_int ("bubblemon/samples", bm->samples);

  gnome_config_pop_prefix ();

  gnome_config_sync ();
  gnome_config_drop_all ();

  return FALSE;
} /* bubblemon_session_save */

void
bubblemon_session_defaults(BubbleMonData * bm)
{
  /* Global configurable parameters */
  bm->update = atoi (BUBBLEMON_DEFAULT_UPDATE_RATE);
  bm->samples = atoi (BUBBLEMON_DEFAULT_LOAD_SAMPLES);

  sscanf (BUBBLEMON_DEFAULT_AIR_NOSWAP, "#%x", &(bm->air_noswap));
  sscanf (BUBBLEMON_DEFAULT_LIQUID_NOSWAP, "#%x", &(bm->liquid_noswap));
  sscanf (BUBBLEMON_DEFAULT_AIR_MAXSWAP, "#%x", &(bm->air_maxswap));
  sscanf (BUBBLEMON_DEFAULT_LIQUID_MAXSWAP, "#%x", &(bm->liquid_maxswap));
}
