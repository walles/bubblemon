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
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
/* #include <config.h> */
#include <gnome.h>
#include <gdk/gdkx.h>

#include <glibtop.h>
#include <glibtop/cpu.h>

#include <applet-widget.h>

#include "bubblemon.h"
#include "session.h"
#include "properties.h"

int
main (int argc, char ** argv)
{
  const gchar *goad_id;
  GtkWidget *applet;

  applet_widget_init ("bubblemon_applet", VERSION, argc, argv, NULL, 0, NULL);
  applet_factory_new ("bubblemon_applet", NULL,
		     (AppletFactoryActivator) applet_start_new_applet);

  goad_id = goad_server_activation_id ();
  if (! goad_id)
    exit(EXIT_FAILURE);

  /* Create the bubblemon applet widget */
  applet = make_new_bubblemon_applet (goad_id);

  /* Run... */
  applet_widget_gtk_main ();

  return 0;
} /* main */

#define SPARK_EDGE 4

int total;

/*
 * This function, bubblemon_update, gets the CPU usage and updates
 * the fire array and pixmap.
 *
 * FIXME: This function should bubble instead of burn.
 */
gint
bubblemon_update (gpointer data)
{
  BubbleMonData * mc = data;
  int i, w, h, n, bytesPerPixel, percent, *buf, *col, x, y;
  int aircolor, watercolor, waterlevel;
  glibtop_cpu cpu;
  uint64_t load, total, oLoad, oTotal;

  // mc->setup is a status byte that is true if we are rolling
  if (!mc->setup)
    return FALSE;

  // Find out the CPU load
  glibtop_get_cpu (&cpu);
  load = cpu.user + cpu.nice + cpu.sys;
  total = cpu.total;
  
  // "i" is an index into a load history
  i = mc->loadIndex;
  oLoad = mc->load[i];
  oTotal = mc->total[i];

  mc->load[i] = load;
  mc->total[i] = total;
  mc->loadIndex = (i + 1) % mc->samples;

  // FIXME: Is the comment on the next line correct?
  // Because the load returned from libgtop is a value accumulated
  // over time, and not the current load, the current load percentage
  // is calculated as the extra amount of work that has been performed
  // since the last sample.
  // FIXME: Shouldn't (total - oTotal) be != 0 instead of just oTotal
  // as on the next line?  Or does oTotal==0 simply imply that this is
  // the first time we execute the current function?
  if (oTotal == 0)
    percent = 0;
  else
    percent = 100 * (load - oLoad) / (total - oTotal);

  // The buf is made up of ints (0-(NUM_COLOURS-1)), each pointing out
  // an entry in the color table.  A pixel in the buf is accessed
  // using the formula buf[row * w + column].
  buf = mc->firebuf;
  col = mc->colours;
  w = mc->breadth;
  h = mc->depth;
  n = w * h;

  // FIXME: The colors of air and water should vary with how many
  // percent of the available swap space that is in use.
  aircolor = 0;
  watercolor = 1;

  // Set the water level depending on the system load
  waterlevel = h - ((h * percent) / 100);

  // Here comes the fire magic.  Pixels are drawn by setting values in
  // buf to 0-NUM_COLORS.  We should possibly make some macros or
  // inline functions to {g|s}et pixels.
  for (x = 0; x < w; x++)
    for (y = 0; y < h; y++)
      {
	if (y < waterlevel)
	  buf[y * w + x] = aircolor;
	else
	  buf[y * w + x] = watercolor;
      }
  
/*    for (i = 0; i < (percent >> 3) + 2; ++ i) */
/*      buf[SPARK_EDGE + (random () % (w - 2 * SPARK_EDGE)) + n] = random () % NUM_COLOURS; */
/*    for (i = 0; i < (100 - percent) >> 4; ++ i) */
/*      buf[SPARK_EDGE + (random () % (w - 2 * SPARK_EDGE)) + n] >>= 1; */
/*    for (i = n - 1; i >= 0; -- i) */
/*      buf[i] = (buf[i + w - 1] + buf[i + w] + buf[i + w + 1] + buf[i]) >> 2; */

  bytesPerPixel = GDK_IMAGE_XIMAGE (mc->image)->bytes_per_line / w;

  // Copy the fire image data to the gdk image
  switch (bytesPerPixel) {
    case 4: {
      uint32_t *ptr = (uint32_t *) GDK_IMAGE_XIMAGE (mc->image)->data;
      for (i = 0; i < n; ++ i)
        ptr[i] = col[buf[i]];
      break;
    }
    case 2: {
      uint16_t *ptr = (uint16_t *) GDK_IMAGE_XIMAGE (mc->image)->data;
      for (i = 0; i < n; ++ i)
        ptr[i] = col[buf[i]];
      break;
    }
  }

  /* Update the display. */
  bubblemon_expose_handler (mc->area, NULL, mc);

  bubblemon_set_timeout (mc);

  return TRUE;
} /* bubblemon_update */


/*
 * This function, bubblemon_expose, is called whenever a portion of the
 * applet window has been exposed and needs to be redrawn.  In this
 * function, we just blit the appropriate portion of the pixmap onto the window.
 *
 */
gint
bubblemon_expose_handler (GtkWidget * ignored, GdkEventExpose * expose,
			gpointer data)
{
  BubbleMonData * mc = data;

  if (!mc->setup)
    return FALSE;

  gdk_draw_image (mc->area->window, mc->area->style->fg_gc[GTK_WIDGET_STATE (mc->area)],
                  mc->image, 0, 0, 0, 0, mc->breadth, mc->depth);
  
  return FALSE; 
} /* bubblemon_expose_handler */

gint
bubblemon_configure_handler (GtkWidget *widget, GdkEventConfigure *event,
			   gpointer data)
{
  BubbleMonData * mc = data;
  
  bubblemon_update ( (gpointer) mc);

  return TRUE;
}  /* bubblemon_configure_handler */

GtkWidget *
applet_start_new_applet (const gchar *goad_id, const char **params,
			 int nparams)
{
  return make_new_bubblemon_applet (goad_id);
} /* applet_start_new_applet */

gint
bubblemon_delete (gpointer data) {
  BubbleMonData * mc = data;

  mc->setup = FALSE;

  if (mc->timeout) {
    gtk_timeout_remove (mc->timeout);
    mc->timeout = 0;
  }

  applet_widget_gtk_main_quit();
}

/* This is the function that actually creates the display widgets */
GtkWidget *
make_new_bubblemon_applet (const gchar *goad_id)
{
  BubbleMonData * mc;
  gchar * param = "bubblemon_applet";

  mc = g_new0 (BubbleMonData, 1);

  mc->applet = applet_widget_new (goad_id);

  if (!glibtop_init_r (&glibtop_global_server, 0, 0))
    g_error (_("Can't open glibtop!\n"));
  
  if (mc->applet == NULL)
    g_error (_("Can't create applet!\n"));

  /*
   * Load all the saved session parameters (or the defaults if none
   * exist).
   */
  if ( (APPLET_WIDGET (mc->applet)->privcfgpath) &&
       * (APPLET_WIDGET (mc->applet)->privcfgpath))
    bubblemon_session_load (APPLET_WIDGET (mc->applet)->privcfgpath, mc);
  else
    bubblemon_session_defaults (mc);

  /*
   * area is the drawing area into which the little picture of
   * the bubblemon gets drawn.
   */
  mc->area = gtk_drawing_area_new ();
  gtk_widget_set_usize (mc->area, mc->breadth, mc->depth);

  /* Set up the event callbacks for the area. */
  gtk_signal_connect (GTK_OBJECT (mc->area), "expose_event",
		      (GtkSignalFunc)bubblemon_expose_handler, mc);
  gtk_widget_set_events (mc->area, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

  applet_widget_add (APPLET_WIDGET (mc->applet), mc->area);

  gtk_signal_connect (GTK_OBJECT (mc->applet), "save_session",
		      GTK_SIGNAL_FUNC (bubblemon_session_save),
		      mc);

  gtk_signal_connect (GTK_OBJECT (mc->applet), "delete_event",
                      GTK_SIGNAL_FUNC (bubblemon_delete),
                      mc);

  applet_widget_register_stock_callback (APPLET_WIDGET (mc->applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _("About..."),
					 about_cb,
					 mc);

  applet_widget_register_stock_callback (APPLET_WIDGET (mc->applet),
					 "properties",
					 GNOME_STOCK_MENU_PROP,
					 ("Properties..."),
					 bubblemon_properties_window,
					 mc);

  gtk_widget_show_all (mc->applet);

  /* Size things according to the saved settings. */
  bubblemon_set_size (mc);

  bubblemon_setup_samples (mc);

  bubblemon_setup_colours (mc);

  /* Nothing is drawn until this is set. */
  mc->setup = TRUE;

  /* Will schedule a timeout automatically */
  bubblemon_update (mc);

  return mc->applet;
} /* make_new_bubblemon_applet */

void bubblemon_set_timeout (BubbleMonData *mc) { 
  gint when = mc->update;
  
  if (when != mc->timeout_t) {
    if (mc->timeout) {
      gtk_timeout_remove (mc->timeout);
      mc->timeout = 0;
    }
    mc->timeout_t = when;
    mc->timeout = gtk_timeout_add (when, (GtkFunction) bubblemon_update, mc);
  }
}

void bubblemon_setup_samples (BubbleMonData *mc) {
  int i;
  uint64_t load = 0, total = 0;

  if (mc->load) {
    load = mc->load[mc->loadIndex];
    free (mc->load);
  }

  if (mc->total) {
    total = mc->total[mc->loadIndex];
    free (mc->total);
  }

  mc->loadIndex = 0;
  mc->load = malloc (mc->samples * sizeof (uint64_t));
  mc->total = malloc (mc->samples * sizeof (uint64_t));
  for (i = 0; i < mc->samples; ++ i) {
    mc->load[i] = load;
    mc->total[i] = total;
  }
}

void bubblemon_setup_colours (BubbleMonData *mc) {
  int i, *col;
  GdkColormap *golormap;
  Display *display;
  Colormap colormap;

  golormap = gdk_colormap_get_system ();
  display = GDK_COLORMAP_XDISPLAY(golormap);
  colormap = GDK_COLORMAP_XCOLORMAP(golormap);

  if (!mc->colours)
    mc->colours = malloc (NUM_COLOURS * sizeof (int));
  col = mc->colours;
  
  for (i = 0; i < NUM_COLOURS; ++ i) {
    int r, g, b;
    char rgbStr[24];
    XColor exact, screen;

    r = (bubblemon_flame[i] >> 16) & 0xff;
    g = (bubblemon_flame[i] >> 8) & 0xff;
    b = bubblemon_flame[i] & 0xff;

    sprintf (rgbStr, "rgb:%.2x/%.2x/%.2x", r, g, b);
    
    XAllocNamedColor (display, colormap, rgbStr, &exact, &screen);
    
    col[i] = screen.pixel;
  }
}

void
destroy_about (GtkWidget *w, gpointer data)
{
  BubbleMonData *mc = data;
} /* destroy_about */

void
about_cb (AppletWidget *widget, gpointer data)
{
  BubbleMonData *mc = data;
  char *authors[2];
  
  authors[0] = "Johan Walles <d92-jwa@nada.kth.se>";
  authors[1] = NULL;

  mc->about_box =
    gnome_about_new (_("Bubbling Load Monitor"), VERSION,
		     _("Copyright (C) 1999 Johan Walles"),
		     (const char **) authors,
	     _("This applet displays your CPU load as a fire.  "
	       "GNOME code ripped from Merlin Hughes' Merlin's CPU Fire Applet.  "
               "Fire code ripped from Zinx Verituse' cpufire.  "
               "This applet comes with ABSOLUTELY NO WARRANTY.  "
               "See the LICENSE file for details.\n"
               "This is free software, and you are welcome to redistribute it "
               "under certain conditions.  "
               "See the LICENSE file for details.\n"),
		     NULL);

  gtk_signal_connect (GTK_OBJECT (mc->about_box), "destroy",
		      GTK_SIGNAL_FUNC (destroy_about), mc);

  gtk_widget_show (mc->about_box);
} /* about_cb */

void
bubblemon_set_size (BubbleMonData * mc)
{
  int bpp;

  gtk_widget_set_usize (mc->area, mc->breadth, mc->depth);

  if (mc->firebuf)
    free (mc->firebuf);

  mc->firebuf = malloc (mc->breadth * (mc->depth + 1) * sizeof (int));
  memset (mc->firebuf, 0, mc->breadth * (mc->depth + 1) * sizeof (int));

  /*
   * If the image has already been allocated, then free them here
   * before creating a new one.  */
  if (mc->image)
    gdk_image_destroy (mc->image);

  mc->image = gdk_image_new (GDK_IMAGE_SHARED, gtk_widget_get_visual (mc->area), mc->breadth, mc->depth);

  bpp = GDK_IMAGE_XIMAGE (mc->image)->bytes_per_line / mc->breadth;

  if ((bpp != 2) && (bpp != 4))
    gnome_error_dialog (_("Bubbling Load Monitor:\nOnly 16bpp and 32bpp modes are supported!\n"));
} /* bubblemon_set_size */
