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

#ifndef _BUBBLEMON_H
#define _BUBBLEMON_H

/* I dunno what PACKAGE means... */
#define VERSION "0.1.1"

#include <applet-widget.h>

#define NUM_COLOURS 256

typedef struct BubbleMonData {
  GtkWidget *applet;

  gint breadth, depth, update, samples;
  int *firebuf, *colours;

  gboolean setup;

  guint timeout;
  gint timeout_t;

  int loadIndex;
  uint64_t *load, *total;
  
  gchar background_s[24];
  GdkColor background;

  /* Widgets n stuff... */
  GtkWidget *area;
  GdkImage *image;
  GtkWidget *about_box;

  /*
   *
   * For the  "Properties" window ...
   *
   */
  GnomePropertyBox *prop_win;
  GtkObject *breadth_adj, *depth_adj, *update_adj, *samples_adj;
  GnomeColorPicker *background_sel;

} BubbleMonData;

/*
 *
 * Configuration defaults
 *
 */

/* Global configuration parameters */
#define BUBBLEMON_DEFAULT_BREADTH         "48"
#define BUBBLEMON_DEFAULT_DEPTH           "48"
#define BUBBLEMON_DEFAULT_UPDATE_RATE     "20"
#define BUBBLEMON_DEFAULT_LOAD_SAMPLES    "16"
#define BUBBLEMON_DEFAULT_BACKGROUND      "#4C0000"

/*
 *
 * Prototypes
 *
 */
void about_cb (AppletWidget *widget, gpointer data);
void destroy_about(GtkWidget *w, gpointer data);

void bubblemon_setup_samples(BubbleMonData *mc);
void bubblemon_setup_colours(BubbleMonData *mc);
void bubblemon_set_size(BubbleMonData *mc);
void bubblemon_set_timeout(BubbleMonData *mc);
gint bubblemon_update(gpointer data);
gint bubblemon_orient_handler(GtkWidget *w, PanelOrientType o,
				   gpointer data);
gint bubblemon_configure_handler(GtkWidget *widget, GdkEventConfigure *event,
			       gpointer data);
GtkWidget *make_new_bubblemon_applet (const gchar *goad_id);
GtkWidget *applet_start_new_applet (const gchar *goad_id,
				     const char **params, int nparams);

extern int bubblemon_flame[];

#endif /* _BUBBLEMON_H */
