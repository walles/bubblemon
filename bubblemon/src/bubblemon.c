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

#include <config.h>

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gnome.h>
#include <gdk/gdkx.h>

#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>

#include <applet-widget.h>

#include "bubblemon.h"
#include "session.h"

#ifdef ENABLE_PROFILING
char *program_name = NULL;
#endif

int
main (int argc, char ** argv)
{
  const gchar *goad_id;
  GtkWidget *applet;

  applet_widget_init ("bubblemon_applet", VERSION, argc, argv, NULL, 0, NULL);
  applet_factory_new ("bubblemon_applet", NULL,
		     (AppletFactoryActivator) applet_start_new_applet);

#ifdef ENABLE_PROFILING
  program_name = strdup(argv[0]);

  fprintf(stderr,
	  PACKAGE " has been configured with --enable-profiling and will terminate in\n"
	  "roughly one minute.  Let's try to make it as representative of normal use\n"
	  "as possible, shall we?\n");
#endif

  goad_id = goad_server_activation_id ();
  if (! goad_id)
    exit(EXIT_FAILURE);

  /* Create the bubblemon applet widget */
  applet = make_new_bubblemon_applet (goad_id);

  /* Run... */
  applet_widget_gtk_main ();

  return 0;
} /* main */

int get_cpu_load(BubbleMonData *bm)  /* Returns the current CPU load in percent */
{
  glibtop_cpu cpu;
  int loadPercentage;
  uint64_t load, total, oLoad, oTotal;
  int i;

  /* Find out the CPU load */
  glibtop_get_cpu (&cpu);
  load = cpu.user + cpu.nice + cpu.sys;
  total = cpu.total;
  
  /* "i" is an index into a load history */
  i = bm->loadIndex;
  oLoad = bm->load[i];
  oTotal = bm->total[i];

  bm->load[i] = load;
  bm->total[i] = total;
  bm->loadIndex = (i + 1) % bm->samples;

  /* FIXME: Is the next comment correct? */
  /*
    Because the load returned from libgtop is a value accumulated
    over time, and not the current load, the current load percentage
    is calculated as the extra amount of work that has been performed
    since the last sample.
  */
  /*
    FIXME: Shouldn't (total - oTotal) be != 0 instead of just oTotal
    as on the next line?  Or does oTotal==0 simply imply that this is
    the first time we execute the current function?
  */
  if (oTotal == 0)
    loadPercentage = 0;
  else
    loadPercentage = 100 * (load - oLoad) / (total - oTotal);

  return loadPercentage;
}

void get_memory_load(BubbleMonData *bm,
		     int *memoryPercentage,
		     int *swapPercentage)
{
  glibtop_mem memory;
  glibtop_swap swap;

  uint64_t add, substract;

  static int swap_delay = 0;
  static uint64_t swap_used = 0, swap_total = 0;

  glibtop_get_mem (&memory);

  /*
    Find out the swap load, but update it only every 50 times we get
    here.

    FIXME: I have absolutely no idea how often that is.
  */
  if (swap_delay <= 0)
    {
      glibtop_get_swap (&swap);

      /* FIXME: Why is this necessary? */
      swap_used = swap.used;
      swap_total = swap.total;
      
      /*
	FIXME: The following number should be based on a constant or
	variable.
      */
      swap_delay = 50;
    }
  swap_delay--;
  
  /*
    Calculate the projected memory + swap load to show the user.  The
    waterlevel shown to the user is how much memory the system is
    using relative to the total amount of electronic RAM.  The swap
    color indicates how much memory above the amount of electronic
    RAM the system is using.
     
    This scheme does *not* show how the system has decided to
    allocate swap and electronic RAM to the users' processes.
  */
  *memoryPercentage =
    (100 *
     (swap_used + memory.used - memory.cached - memory.buffer)) / memory.total;
  if (*memoryPercentage > 100)
    *memoryPercentage = 100;

  if (*memoryPercentage < 0)
    {
      fprintf(stderr,
	      "memoryPercentage (%d) is out of range (0-100).  It has been calculated\n"
	      "from swap_used (%Ld), memory.used (%Ld), memory.cached (%Ld),\n"
	      "memory.buffer (%Ld), memory.total (%Ld) and swap_total (%Ld).\n",
	      *memoryPercentage,
	      swap_used,
	      memory.used,
	      memory.cached,
	      memory.buffer,
	      memory.total,
	      swap_total);
      exit (EXIT_FAILURE);
    }

  add = swap_used + memory.used;
  substract = memory.cached + memory.buffer + memory.total;

  if (add > substract)
    *swapPercentage = (100 * (add - substract)) / swap_total;
  else
    *swapPercentage = 0;

  if (*swapPercentage > 100)
    {
      fprintf(stderr,
	      "swapPercentage (%d) is out of range (0-100).  It has been calculated from\n"
	      "swap_used (%Ld), memory.used (%Ld), memory.cached (%Ld),\n"
	      "memory.buffer (%Ld), memory.total (%Ld) and swap_total (%Ld).\n",
	      *swapPercentage,
	      swap_used,
	      memory.used,
	      memory.cached,
	      memory.buffer,
	      memory.total,
	      swap_total);
      exit (EXIT_FAILURE);
    }
}

/*
 * This function, bubblemon_update, gets the CPU usage and updates
 * the bubble array and pixmap.
 */
gint
bubblemon_update (gpointer data)
{
  BubbleMonData * bm = data;
  Bubble *bubbles = bm->bubbles;
  int i, w, h, n_pixels, bytesPerPixel, loadPercentage, *buf, *col, x, y;
  int aircolor, watercolor, waterlevel_goal, memoryPercentage, bias;
  int swapPercentage;
  int *temp;
  
#ifdef ENABLE_PROFILING
  static int profiling_countdown = 2500;  /* FIXME: Is 250 calls to here == 5 seconds? */
  
  if (profiling_countdown-- < 0)
    {
      /*
	We terminate after a little while so we don't have to wait
	forever for the profiling data to appear.
      */
      char *home;
      
      /* Change directory to the user's home directory */
      home = getenv("HOME");
      if (home)
	{
	  if (chdir(home) != 0)
	    {
	      char *errormsg = (char *)malloc(sizeof(char) *
					      (strlen(home) + 100));
	      sprintf(errormsg, "Couldn't chdir() to $HOME (%s)\n", home);
	      perror(errormsg);
	    }
	}
      else
	{
	  fprintf(stderr, "Error: $HOME environment variable not set\n");
	}
      
      /* Terminate nicely so that the profiling data gets written */
      fprintf(stderr,
	      "Bubblemon exiting.  Profiling data should be in ~/gmon.out.\n"
	      "For a good time, run 'gprof -l -p %s ~/gmon.out'.\n",
	      (program_name?program_name:"<name of executable>"));
      exit(EXIT_SUCCESS);
    }
#endif  /* ENABLE_PROFILING */
  
  /* bm->setup is a status byte that is true if we are rolling */
  if (!bm->setup)
    return FALSE;

  /* Find out the CPU load */
  loadPercentage = get_cpu_load(bm);

  /* Find out the memory load */
  get_memory_load(bm, &memoryPercentage, &swapPercentage);

  /*
    The buf is made up of ints (0-(NUM_COLORS-1)), each pointing out
    an entry in the color table.  A pixel in the buf is accessed
    using the formula buf[row * w + column].
  */
  buf = bm->bubblebuf;
  col = bm->colors;
  w = bm->breadth;
  h = bm->depth;
  n_pixels = w * h;

  /*
    Vary the colors of air and water with how many
    percent of the available swap space that is in use.
  */
  aircolor = ((((NUM_COLORS >> 1) - 1) * swapPercentage) / 100) << 1;
  watercolor = aircolor + 1;

  /* Sanity check the colors */
  if ((aircolor < 0) || (watercolor >= NUM_COLORS))
    {
      fprintf(stderr,
	      "Error: aircolor (%d) or watercolor (%d) out of bounds (0-%d).\n"
	      "       swapPercentage (%d) is probably out of range (0-100) too.\n",
	      aircolor,
	      watercolor,
	      NUM_COLORS,
	      swapPercentage);
      exit(EXIT_FAILURE);
    }

  /* Move the water level with the current memory usage. */
  waterlevel_goal = h - ((memoryPercentage * h) / 100);

  /* Update the waterlevels */
  bm->waterlevels[0] = waterlevel_goal;
  bm->waterlevels[w - 1] = waterlevel_goal;

  for (x = 1; x < w; x++)
    {
      bm->waterlevels_inactive[x] = (bm->waterlevels[x - 1] +
				     bm->waterlevels[x + 1]) >> 1;

      if (bm->waterlevels_inactive[x] == bm->waterlevels_inactive[x])
	{
	  /* This waterlevel hasn't moved; guard from rounding errors */
	  bias = (x < (w >> 1)) ? (x - 1) : (x + 1);
	  if (bm->waterlevels_inactive[x] < bm->waterlevels[bias])
	    bm->waterlevels_inactive[x]++;
	}
    }

  bm->waterlevels_inactive[0] = waterlevel_goal;
  bm->waterlevels_inactive[w - 1] = waterlevel_goal;

  temp = bm->waterlevels_inactive;
  bm->waterlevels_inactive = bm->waterlevels;
  bm->waterlevels = temp;

  /*
    Here comes the bubble magic.  Pixels are drawn by setting values in
    buf to 0-NUM_COLORS.  We should possibly make some macros or
    inline functions to {g|s}et pixels.
  */

  /* Draw the air-and-water background */
  for (x = 0; x < w; x++)
    for (y = 0; y < h; y++)
      {
	if (y < bm->waterlevels[x])
	  buf[y * w + x] = aircolor;
	else
	  buf[y * w + x] = watercolor;
      }

  /* Create a new bubble if the planets are correctly aligned... */
  if ((bm->n_bubbles < MAX_BUBBLES) && ((random() % 101) <= loadPercentage))
    {
      bubbles[bm->n_bubbles].x = random() % w;
      bubbles[bm->n_bubbles].y = h - 1;
      bubbles[bm->n_bubbles].dy = 0.0;
      bm->n_bubbles++;
    }
  
  /* Update and draw the bubbles */
  for (i = 0; i < bm->n_bubbles; i++)
    {
      /* Accellerate the bubble */
      bubbles[i].dy -= GRAVITY;

      /* Move the bubble vertically */
      bubbles[i].y += bubbles[i].dy;

      /* Did we lose it? */
      if (bubbles[i].y < bm->waterlevels[bubbles[i].x])
	{
	  /* Yes; nuke it */
	  bubbles[i].x  = bubbles[bm->n_bubbles - 1].x;
	  bubbles[i].y  = bubbles[bm->n_bubbles - 1].y;
	  bubbles[i].dy = bubbles[bm->n_bubbles - 1].dy;
	  bm->n_bubbles--;

	  /*
	    We must check the previously last bubble, which is
	    now the current bubble, also.
	  */
	  i--;
	  continue;
	}

      /* Draw the bubble */
      x = bubbles[i].x;
      y = bubbles[i].y;
      
      buf[y * w + x] = aircolor;
    }
  
  /* Drawing magic resides below this point */
  bytesPerPixel = GDK_IMAGE_XIMAGE (bm->image)->bytes_per_line / w;

  /* Copy the bubbling image data to the gdk image */
  switch (bytesPerPixel)
    {
    case 4:
      {
	uint32_t *ptr = (uint32_t *) GDK_IMAGE_XIMAGE (bm->image)->data;
	for (i = 0; i < n_pixels; ++ i)
	  ptr[i] = col[buf[i]];
	break;
      }
      
    case 2:
      {
	uint16_t *ptr = (uint16_t *) GDK_IMAGE_XIMAGE (bm->image)->data;
	for (i = 0; i < n_pixels; ++ i)
	  ptr[i] = col[buf[i]];
	break;
      }

    default:
      fprintf(stderr,
	      "Error: Bubblemon works only on displays with 2 or 4 bytes/pixel :-(.\n"
	      "      If you know how to fix this, please let me (d92-jwa@nada.kth.se\n"
	      "      know).  The fix should probably go into %s, just above line %d.\n",
	      __FILE__,
	      __LINE__);
      exit(EXIT_FAILURE);
  }

  /* Update the display. */
  bubblemon_expose_handler (bm->area, NULL, bm);

  bubblemon_set_timeout (bm);

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
  BubbleMonData * bm = data;

  if (!bm->setup)
    return FALSE;

  gdk_draw_image (bm->area->window, bm->area->style->fg_gc[GTK_WIDGET_STATE (bm->area)],
                  bm->image, 0, 0, 0, 0, bm->breadth, bm->depth);
  
  return FALSE; 
} /* bubblemon_expose_handler */

gint
bubblemon_configure_handler (GtkWidget *widget, GdkEventConfigure *event,
			   gpointer data)
{
  BubbleMonData * bm = data;
  
  bubblemon_update ( (gpointer) bm);

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
  BubbleMonData * bm = data;

  bm->setup = FALSE;

  if (bm->timeout) {
    gtk_timeout_remove (bm->timeout);
    bm->timeout = 0;
  }

  applet_widget_gtk_main_quit();

  return TRUE;  /* We do our own destruction */
}

/* This is the function that actually creates the display widgets */
GtkWidget *
make_new_bubblemon_applet (const gchar *goad_id)
{
  BubbleMonData * bm;
  GtkWidget *frame;

  bm = g_new0 (BubbleMonData, 1);

  bm->applet = applet_widget_new (goad_id);

  if (!glibtop_init_r (&glibtop_global_server, 0, 0))
    g_error (_("Can't open glibtop!\n"));
  
  if (bm->applet == NULL)
    g_error (_("Can't create applet!\n"));

  /*
   * Load all the saved session parameters (or the defaults if none
   * exist).
   */
  if ( (APPLET_WIDGET (bm->applet)->privcfgpath) &&
       * (APPLET_WIDGET (bm->applet)->privcfgpath))
    bubblemon_session_load (APPLET_WIDGET (bm->applet)->privcfgpath, bm);
  else
    bubblemon_session_defaults (bm);

  /* We begin with zero bubbles */
  bm->n_bubbles = 0;
  
  /*
   * area is the drawing area into which the little picture of
   * the bubblemon gets drawn.
   */
  bm->area = gtk_drawing_area_new ();
  gtk_widget_set_usize (bm->area, bm->breadth, bm->depth);

  /* frame is the frame around the drawing area */
  frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);

  /* frame the drawing area */
  gtk_container_add(GTK_CONTAINER(frame), bm->area);
  
  /* Set up the event callbacks for the area. */
  gtk_signal_connect (GTK_OBJECT (bm->area), "expose_event",
		      (GtkSignalFunc)bubblemon_expose_handler, bm);
  gtk_widget_set_events (bm->area, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

  applet_widget_add (APPLET_WIDGET (bm->applet), frame);

  gtk_signal_connect (GTK_OBJECT (bm->applet), "save_session",
		      GTK_SIGNAL_FUNC (bubblemon_session_save),
		      bm);

  gtk_signal_connect (GTK_OBJECT (bm->applet), "delete_event",
                      GTK_SIGNAL_FUNC (bubblemon_delete),
                      bm);

  applet_widget_register_stock_callback (APPLET_WIDGET (bm->applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _("About..."),
					 about_cb,
					 bm);

  gtk_widget_show_all (bm->applet);

  /* Size things according to the saved settings. */
  bubblemon_set_size (bm);

  bubblemon_setup_samples (bm);

  bubblemon_setup_colors (bm);

  /* Nothing is drawn until this is set. */
  bm->setup = TRUE;

  /* Will schedule a timeout automatically */
  bubblemon_update (bm);

  return bm->applet;
} /* make_new_bubblemon_applet */

void bubblemon_set_timeout (BubbleMonData *bm) { 
  gint when = bm->update;
  
  if (when != bm->timeout_t) {
    if (bm->timeout) {
      gtk_timeout_remove (bm->timeout);
      bm->timeout = 0;
    }
    bm->timeout_t = when;
    bm->timeout = gtk_timeout_add (when, (GtkFunction) bubblemon_update, bm);
  }
}

void bubblemon_setup_samples (BubbleMonData *bm) {
  int i;
  uint64_t load = 0, total = 0;

  if (bm->load) {
    load = bm->load[bm->loadIndex];
    free (bm->load);
  }

  if (bm->total) {
    total = bm->total[bm->loadIndex];
    free (bm->total);
  }

  bm->loadIndex = 0;
  bm->load = malloc (bm->samples * sizeof (uint64_t));
  bm->total = malloc (bm->samples * sizeof (uint64_t));
  for (i = 0; i < bm->samples; ++ i) {
    bm->load[i] = load;
    bm->total[i] = total;
  }
}

void bubblemon_setup_colors (BubbleMonData *bm) {
  int i, j, *col;
  int r_air_noswap, g_air_noswap, b_air_noswap;
  int r_liquid_noswap, g_liquid_noswap, b_liquid_noswap;
  int r_air_maxswap, g_air_maxswap, b_air_maxswap;
  int r_liquid_maxswap, g_liquid_maxswap, b_liquid_maxswap;

  GdkColormap *golormap;
  Display *display;
  Colormap colormap;

  golormap = gdk_colormap_get_system ();
  display = GDK_COLORMAP_XDISPLAY(golormap);
  colormap = GDK_COLORMAP_XCOLORMAP(golormap);

  if (!bm->colors)
    bm->colors = malloc (NUM_COLORS * sizeof (int));
  col = bm->colors;

  r_air_noswap = (bm->air_noswap >> 16) & 0xff;
  g_air_noswap = (bm->air_noswap >> 8) & 0xff;
  b_air_noswap = (bm->air_noswap) & 0xff;

  r_liquid_noswap = (bm->liquid_noswap >> 16) & 0xff;
  g_liquid_noswap = (bm->liquid_noswap >> 8) & 0xff;
  b_liquid_noswap = (bm->liquid_noswap) & 0xff;
  
  r_air_maxswap = (bm->air_maxswap >> 16) & 0xff;
  g_air_maxswap = (bm->air_maxswap >> 8) & 0xff;
  b_air_maxswap = (bm->air_maxswap) & 0xff;

  r_liquid_maxswap = (bm->liquid_maxswap >> 16) & 0xff;
  g_liquid_maxswap = (bm->liquid_maxswap >> 8) & 0xff;
  b_liquid_maxswap = (bm->liquid_maxswap) & 0xff;
  
  for (i = 0; i < NUM_COLORS; ++ i) {
    int r, g, b;
    char rgbStr[24];
    XColor exact, screen;

    if (i & 1)
      {
	/* Liquid */
	j = (i - 1) >> 1;
	
	r = (r_liquid_maxswap * j + r_liquid_noswap * (((NUM_COLORS >> 1) - 1) - j)) / ((NUM_COLORS >> 1) - 1);
	g = (g_liquid_maxswap * j + g_liquid_noswap * (((NUM_COLORS >> 1) - 1) - j)) / ((NUM_COLORS >> 1) - 1);
	b = (b_liquid_maxswap * j + b_liquid_noswap * (((NUM_COLORS >> 1) - 1) - j)) / ((NUM_COLORS >> 1) - 1);
      }
    else
      {
	/* Air */
	j = i >> 1;

	r = (r_air_maxswap * j + r_air_noswap * (((NUM_COLORS >> 1) - 1) - j)) / ((NUM_COLORS >> 1) - 1);
	g = (g_air_maxswap * j + g_air_noswap * (((NUM_COLORS >> 1) - 1) - j)) / ((NUM_COLORS >> 1) - 1);
	b = (b_air_maxswap * j + b_air_noswap * (((NUM_COLORS >> 1) - 1) - j)) / ((NUM_COLORS >> 1) - 1);
      }

    sprintf (rgbStr, "rgb:%.2x/%.2x/%.2x", r, g, b);
    
    XAllocNamedColor (display, colormap, rgbStr, &exact, &screen);
    
    col[i] = screen.pixel;
  }
}

void
destroy_about (GtkWidget *w, gpointer data)
{
} /* destroy_about */

void
about_cb (AppletWidget *widget, gpointer data)
{
  BubbleMonData *bm = data;
  char *authors[2];
  
  authors[0] = "Johan Walles <d92-jwa@nada.kth.se>";
  authors[1] = NULL;

  bm->about_box =
    gnome_about_new (_("Bubbling Load Monitor"), VERSION,
		     _("Copyright (C) 1999 Johan Walles"),
		     (const char **) authors,
	     _("This applet displays your CPU load as a bubbling liquid.  "
	       "GNOME code ripped from Merlin Hughes' Merlin's CPU Fire Applet.  "
               "This applet comes with ABSOLUTELY NO WARRANTY.  "
               "See the LICENSE file for details.\n"
               "This is free software, and you are welcome to redistribute it "
               "under certain conditions.  "
               "See the LICENSE file for details.\n"),
		     NULL);

  gtk_signal_connect (GTK_OBJECT (bm->about_box), "destroy",
		      GTK_SIGNAL_FUNC (destroy_about), bm);

  gtk_widget_show (bm->about_box);
} /* about_cb */

void
bubblemon_set_size (BubbleMonData * bm)
{
  int bpp, i;

  gtk_widget_set_usize (bm->area, bm->breadth, bm->depth);

  if (bm->bubblebuf)
    free (bm->bubblebuf);

  bm->bubblebuf = malloc (bm->breadth * (bm->depth + 1) * sizeof (int));
  memset (bm->bubblebuf, 0, bm->breadth * (bm->depth + 1) * sizeof (int));

  if (bm->waterlevels)
    free (bm->waterlevels);

  bm->waterlevels = malloc (bm->breadth * sizeof (int));
  for (i = 0; i < bm->breadth; i++)
    {
      bm->waterlevels[i] = bm->depth;
    }

  if (bm->waterlevels_inactive)
    free (bm->waterlevels_inactive);

  bm->waterlevels_inactive = malloc (bm->breadth * sizeof (int));

  /*
   * If the image has already been allocated, then free them here
   * before creating a new one.  */
  if (bm->image)
    gdk_image_destroy (bm->image);

  bm->image = gdk_image_new (GDK_IMAGE_SHARED, gtk_widget_get_visual (bm->area), bm->breadth, bm->depth);

  bpp = GDK_IMAGE_XIMAGE (bm->image)->bytes_per_line / bm->breadth;

  if ((bpp != 2) && (bpp != 4))
    gnome_error_dialog (_("Bubbling Load Monitor:\nOnly 16bpp and 32bpp modes are supported!\n"));
} /* bubblemon_set_size */
