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

#include <applet-widget.h>
#include <sys/types.h>

// Note: NUM_COLORS must be divisible by 3
#define NUM_COLORS 99 /* 384 */
#define MAX_BUBBLES 100

// How fast do the bubbles rise?
#define GRAVITY 0.01

// How fast do the water levels accelerate?
#define VOLATILITY 1.0

// 0.0 means the liquid never moves.  1.0 means the liquid will continue
// to oscillate forever.
#define VISCOSITY 0.92

// How fast are the water levels allowed to move?
#define SPEED_LIMIT 1.0

typedef struct {
  int x;    /* Horizontal coordinate */
  float y;  /* Vertical coordinate   */
  float dy; /* Vertical velocity     */

  /* FIXME: Should we give every bubble a radius? */
} Bubble;

typedef struct {
  GtkWidget *applet;

  gint breadth, depth, update, samples;
  int *bubblebuf, *colors;
  float *waterlevels;
  float *waterlevels_dy;

  Bubble bubbles[MAX_BUBBLES];
  int n_bubbles;

  /* Color definitions */
  int air_noswap, liquid_noswap, air_maxswap, liquid_maxswap;

  gboolean setup;

  guint timeout;
  gint timeout_t;

  int loadIndex;
  u_int64_t *load, *total;
  
  /* Widgets n stuff... */
  GtkWidget *area;
  GtkWidget *frame;
  GdkImage *image;
  GtkWidget *about_box;

} BubbleMonData;

/*
 *
 * Configuration defaults
 *
 */

/* Global configuration parameters */
#define BUBBLEMON_DEFAULT_BREADTH         "32"
#define BUBBLEMON_DEFAULT_DEPTH           "40"
#define BUBBLEMON_DEFAULT_UPDATE_RATE     "20"
#define BUBBLEMON_DEFAULT_LOAD_SAMPLES    "16"
/*
  FIXME: There should be three constants for how often the cpu, swap
  and memory loads are updated.
*/

#define BUBBLEMON_DEFAULT_AIR_NOSWAP      "#2299FF"
#define BUBBLEMON_DEFAULT_LIQUID_NOSWAP    "#0055FF"

#define BUBBLEMON_DEFAULT_AIR_MAXSWAP     "#FF0000"
#define BUBBLEMON_DEFAULT_LIQUID_MAXSWAP   "#AA0000"

/*
 *
 * Prototypes
 *
 */
void about_cb (AppletWidget *widget, gpointer data);
/* (Begin) PoolGod 26/06/2000 */
void widget_leave_cb (GtkWidget *, GdkEventAny *, gpointer data);
/* (End) PoolGod */
void destroy_about(GtkWidget *w, gpointer data);

void bubblemon_setup_samples(BubbleMonData *mc);
void bubblemon_setup_colors(BubbleMonData *mc);
void bubblemon_set_size(BubbleMonData *mc);
void bubblemon_set_timeout(BubbleMonData *mc);
gint bubblemon_update(gpointer data);
gint bubblemon_delete(gpointer data);
gint bubblemon_orient_handler(GtkWidget *w, PanelOrientType o,
				   gpointer data);
gint bubblemon_configure_handler(GtkWidget *widget, GdkEventConfigure *event,
			       gpointer data);
gint bubblemon_expose_handler (GtkWidget * ignored, GdkEventExpose * expose,
			       gpointer data);
GtkWidget *make_new_bubblemon_applet (const gchar *goad_id);
GtkWidget *applet_start_new_applet (const gchar *goad_id,
				     const char **params, int nparams);

int get_cpu_load(BubbleMonData *bm);
void usage2string(char *string, u_int64_t used, u_int64_t max);
void get_censored_memory_and_swap(BubbleMonData *bm,
				  u_int64_t *mem_used,
				  u_int64_t *mem_max,
				  u_int64_t *swap_used,
				  u_int64_t *swap_max);
void get_censored_memory_usage(BubbleMonData *bm,
			       u_int64_t *mem_used,
			       u_int64_t *mem_max);
void get_censored_swap_usage(BubbleMonData *bm,
			       u_int64_t *swap_used,
			     u_int64_t *swap_max);
void update_tooltip(BubbleMonData *bm);
void get_memory_load_percentage(BubbleMonData *bm,
				int *memoryPercentage,
				int *swapPercentage);

#endif /* _BUBBLEMON_H */
