/*
 *  Bubbling Load Monitoring Applet
 *  - A GNOME panel applet that displays the CPU + memory load as a
 *    bubbling liquid.
 *  Copyright (C) 1999-2000 Johan Walles
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

#include <config.h>

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
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

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#endif

#ifdef ENABLE_PROFILING
char *program_name = NULL;
#endif

int main (int argc, char ** argv)
{
  const gchar *goad_id;
  GtkWidget *applet;

#ifdef ENABLE_NLS
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif

  applet_widget_init ("bubblemon_applet", VERSION, argc, argv, NULL, 0, NULL);
  applet_factory_new ("bubblemon_applet", NULL,
		     (AppletFactoryActivator) applet_start_new_applet);

  if (NUM_COLORS % 3)
    {
      g_error(_("Error: The NUM_COLORS constant in bubblemon.h must be a multiple of 3.\n"
	      "       The current value of %d is not.\n"),
	      NUM_COLORS);
    }

#ifdef ENABLE_PROFILING
  program_name = strdup(argv[0]);

  g_warning(PACKAGE " has been configured with --enable-profiling and will terminate in\n"
	    "roughly one minute.  Let's try to make it as representative of normal use\n"
	    "as possible, shall we?\n");
#endif

  goad_id = goad_server_activation_id ();
  if (!goad_id)
    {
      /* FIXME: Try starting the bubblemon_applet GOAD server manually
         before failing.  Some hints on doing this can be found at
         "http://developer.gnome.org/doc/API/libgnorba/gnorba-goad.html"
      */
      
      fprintf(stderr,
              "Couldn't activate GOAD server.  This usually means that you are trying\n"
              "to run the applet from the command line and haven't specified the\n"
              "--activate-goad-server=bubblemon_applet switch.  Add that switch and\n"
              "give it another try.\n"
              "\n"
              "If you have any idea about how it can be started from within the code, I'd\n"
              "appreciate it a lot if you could tell me.  Insert your code at the FIXME just\n"
              "above line %d in `%s'.  You'll probably want to RTFM first at\n"
              "`http://developer.gnome.org/doc/API/libgnorba/gnorba-goad.html'.\n"
              "\n"
              "Then send me (d92-jwa@nada.kth.se) an e-mail (in English or Swedish) with your\n"
              "changes.\n"
              "\n"
              "Thanks a bunch :-)  /Johan.\n",
              __LINE__,
              __FILE__);

      exit (EXIT_FAILURE);
    }

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
  u_int64_t load, total, oLoad, oTotal;
  int i;

  /* Find out the CPU load */
  glibtop_get_cpu (&cpu);
  load = cpu.user + cpu.sys;
  total = cpu.total;

  /* "i" is an index into a load history */
  i = bm->loadIndex;
  oLoad = bm->load[i];
  oTotal = bm->total[i];

  bm->load[i] = load;
  bm->total[i] = total;
  bm->loadIndex = (i + 1) % bm->samples;

  /*
    Because the load returned from libgtop is a value accumulated
    over time, and not the current load, the current load percentage
    is calculated as the extra amount of work that has been performed
    since the last sample.
  */
  if (oTotal == 0)  /* oTotal == 0 means that this is the first time
		       we get here */
    loadPercentage = 0;
  else
    loadPercentage = (100 * (load - oLoad)) / (total - oTotal);

  return loadPercentage;
}

void usage2string(char *string,
		  u_int64_t used,
		  u_int64_t max)
{
  /* Create a string of the form "35/64Mb" */
  
  int shiftme = 0;
  char divisor_char = '\0';

  if ((max >> 40) > 0)
    {
      shiftme = 40;
      divisor_char = 'T';
    }
  else if ((max >> 30) > 0)
    {
      shiftme = 30;
      divisor_char = 'G';
    }
  else if ((max >> 20) > 0)
    {
      shiftme = 20;
      divisor_char = 'M';
    }
  else if ((max >> 10) > 0)
    {
      shiftme = 10;
      divisor_char = 'k';
    }

  if (divisor_char)
    {
      sprintf(string, "%Ld/%Ld%cb",
	      used >> shiftme,
	      max >> shiftme,
	      divisor_char);
    }
  else
    {
      sprintf(string, "%Ld/%Ld bytes",
	      used >> shiftme,
	      max >> shiftme);
    }
}

void get_censored_memory_and_swap(BubbleMonData *bm,
				  u_int64_t *mem_used,
				  u_int64_t *mem_max,
				  u_int64_t *swap_used,
				  u_int64_t *swap_max)
{
  static glibtop_mem memory;
  u_int64_t my_mem_used, my_mem_max;
  u_int64_t my_swap_used, my_swap_max;
  
  static glibtop_swap swap;  /* Needs to be static 'cause we don't do it every time */

  static int swap_delay = 0, mem_delay = 0;

  /*
    Find out the memory load, but update it only every 25 times we get
    here.  This works around a performance problem in libgtop that is
    present on some systems, possibly those with > 128Mb memory.

    FIXME: I have absolutely no idea how often this is.
  */
  if (mem_delay <= 0)
    {
      glibtop_get_mem (&memory);
      
      /*
	FIXME: The following number should be based on a constant or
	variable.
      */
      mem_delay = 25;
    }
  mem_delay--;
  
  if (memory.total == 0)
    {
      g_error(_("glibtop_get_mem() says you have no memory on line %d in %s"),
			 __LINE__,
			 __FILE__);
    }
  
  /*
    Find out the swap load, but update it only every 50 times we get
    here.  If we do it every time, it bogs down the program.

    FIXME: I have absolutely no idea how often that is.
  */
  if (swap_delay <= 0)
    {
      glibtop_get_swap (&swap);

      /*
	FIXME: The following number should be based on a constant or
	variable.
      */
      swap_delay = 50;
    }
  swap_delay--;

  /*
    Calculate the projected memory + swap load to show the user.  The
    values given to the user is how much memory the system is using
    relative to the total amount of electronic RAM.  The swap color
    indicates how much memory above the amount of electronic RAM the
    system is using.
    
    This scheme does *not* show how the system has decided to
    allocate swap and electronic RAM to the users' processes.
  */

  my_mem_max = memory.total;
  my_swap_max = swap.total;
  
  my_mem_used = swap.used + memory.used - memory.cached - memory.buffer;

  if (my_mem_used > my_mem_max)
    {
      my_swap_used = my_mem_used - my_mem_max;
      my_mem_used = my_mem_max;
    }
  else
    {
      my_swap_used = 0;
    }

  /* Sanity check that we don't use more swap/mem than what's available */
  g_assert((my_mem_used <= my_mem_max) &&
           (my_swap_used <= my_swap_max));

  *mem_used = my_mem_used;
  *mem_max = my_mem_max;
  *swap_used = my_swap_used;
  *swap_max = my_swap_max;
}

void get_censored_memory_usage(BubbleMonData *bm,
			       u_int64_t *mem_used,
			       u_int64_t *mem_max)
{
  u_int64_t dummy;

  get_censored_memory_and_swap(bm,
			       mem_used, mem_max,
			       &dummy, &dummy);
}

void get_censored_swap_usage(BubbleMonData *bm,
			       u_int64_t *swap_used,
			       u_int64_t *swap_max)
{
  u_int64_t dummy;

  get_censored_memory_and_swap(bm,
			       &dummy, &dummy,
			       swap_used, swap_max);
}

void update_tooltip(BubbleMonData *bm)
{
  char memstring[20], swapstring[20], tooltipstring[200];

  int loadPercentage;

  u_int64_t swap_used;
  u_int64_t swap_max;
  u_int64_t mem_used;
  u_int64_t mem_max;

  /* Sanity check */
  g_assert(bm != NULL);

  get_censored_memory_usage(bm, &mem_used, &mem_max);
  get_censored_swap_usage(bm, &swap_used, &swap_max);
  
  usage2string(memstring, mem_used, mem_max);
  usage2string(swapstring, swap_used, swap_max);

  loadPercentage = get_cpu_load(bm);

  snprintf(tooltipstring, 190,
	   _("Memory used: %s\nSwap used: %s\nCPU load: %d%%"),
	   memstring,
	   swapstring,
	   loadPercentage);

  /* FIXME: How can I prevent the tooltip from being hidden when it's
     re-generated? */

  /* FIXME: This is a workaround for the gtk+ tool tip problem
     described in the TODO file. */
  applet_widget_set_widget_tooltip(APPLET_WIDGET(bm->applet),
				   GTK_WIDGET(bm->area),
				   tooltipstring);
}

void get_memory_load_percentage(BubbleMonData *bm,
				int *memoryPercentage,
				int *swapPercentage)
{
  u_int64_t mem_used;
  u_int64_t mem_max;
  u_int64_t swap_used;
  u_int64_t swap_max;

  get_censored_memory_and_swap(bm,
			       &mem_used, &mem_max,
			       &swap_used, &swap_max);

  g_assert(mem_max > 0);
  
  *memoryPercentage = (100 * mem_used) / mem_max;

  if (swap_max != 0)
    {
      *swapPercentage = (100 * swap_used) / swap_max;
    }
  else
    {
      *swapPercentage = 0;
    }

  /* Sanity check that the percentages are both 0-100. */
  g_assert(((*memoryPercentage >= 0) && (*memoryPercentage <= 100)) &&
           ((*swapPercentage >= 0) && (*swapPercentage <= 100)));
}

/* This function copies the internal image to the screen. */
void update_screen(BubbleMonData *bm,
                   int start_drawing,
                   int stop_drawing)
{
  int i;
  int w;
  int bytesPerPixel;
  int *buf, *col;

  /* FIXME: We should sanity check stxxx_drawing here */
  
  buf = bm->bubblebuf;
  col = bm->colors;
  w = bm->breadth;
  bytesPerPixel = GDK_IMAGE_XIMAGE (bm->image)->bytes_per_line / w;

  start_drawing *= w;
  stop_drawing *= w;

  /* Copy the bubbling image data to the gdk image.  A regular
     memcpy() won't do, because all pixels will have to be looked up
     using col[buf[i]] before being copied to the image buffer. */
  switch (bytesPerPixel)
    {
    case 4:
      {
	u_int32_t *ptr = (u_int32_t *) GDK_IMAGE_XIMAGE (bm->image)->data;
	for (i = start_drawing; i < stop_drawing; i++)
          ptr[i] = col[buf[i]];
	break;
      }

    case 2:
      {
	u_int16_t *ptr = (u_int16_t *) GDK_IMAGE_XIMAGE (bm->image)->data;
	for (i = start_drawing; i < stop_drawing; i++)
	  ptr[i] = col[buf[i]];
	break;
      }

    case 1:
      {
	u_int8_t *ptr = (u_int8_t *) GDK_IMAGE_XIMAGE (bm->image)->data;
	for (i = start_drawing; i < stop_drawing; i++)
	  ptr[i] = col[buf[i]];
	break;
      }

    default:
      g_error("Error: Bubblemon works only on displays with 1 (untested), 2 or 4\n"
	      "      bytes/pixel :-(.  If you know how to fix this, please let me\n"
	      "      (d92-jwa@nada.kth.se) know.  The fix should probably go into %s,\n"
              "      just above line %d.\n",
	      __FILE__,
	      __LINE__);
  }

  /* Update the display. */
  bubblemon_expose_handler (bm->area, NULL, bm);
}

void draw_bubble(BubbleMonData *bm,
                 int x, int y,
                 int aircolor)
{
  int *buf = bm->bubblebuf;
  int w = bm->breadth;
  int h = bm->depth;
  int *buf_ptr;

  /*
    Clipping is not necessary for x, but it *is* for y.
    To prevent ugliness, we draw aliascolor only on top of
    watercolor, and aircolor on top of aliascolor.
  */
  
  /* Top row */
  buf_ptr = &(buf[(y - 1) * w + x - 1]);
  if (y > bm->waterlevels[x])
    {
      if (*buf_ptr != aircolor)
        {
          (*buf_ptr)++ ;
        }
      buf_ptr++;
	  
      *buf_ptr = aircolor; buf_ptr++;
	  
      if (*buf_ptr != aircolor)
        {
          (*buf_ptr)++ ;
        }
      buf_ptr += (w - 2);
    }
  else
    {
      buf_ptr += w;
    }

  /* Middle row - no clipping necessary */
  *buf_ptr = aircolor; buf_ptr++;
  *buf_ptr = aircolor; buf_ptr++;
  *buf_ptr = aircolor; buf_ptr += (w - 2);

  /* Bottom row */
  if (y < (h - 1))
    {
      if (*buf_ptr != aircolor)
        {
          (*buf_ptr)++ ;
        }
      buf_ptr++;
	  
      *buf_ptr = aircolor; buf_ptr++;

      if (*buf_ptr != aircolor)
        {
          (*buf_ptr)++ ;
        }
    }
}

/*
 * This function, bubblemon_update, gets the CPU usage and updates
 * the bubble array and pixmap.
 */
gint bubblemon_update (gpointer data)
{
  BubbleMonData *bm = data;
  Bubble *bubbles = bm->bubbles;
  int i, w, h, n_pixels, loadPercentage, *buf, *buf_ptr, *col, x, y;

  int aircolor, watercolor, aliascolor;

  int swapPercentage, memoryPercentage;

  float waterlevels_goal;
  float current_waterlevel_goal;

  /*
    These values are for keeping track of where we have to start
    drawing water.
  */
  int waterlevel_min, waterlevel_max;
  static int last_waterlevel_min = 0;

  /*
    These values are for keeping track of which pixels in the drawing
    area that we have to update this turn.
  */
  int start_drawing, stop_drawing;
  int action_min = bm->depth, action_max = 0;
  static int last_action_min = 0, last_action_max = -1;

#ifdef ENABLE_PROFILING
  static int profiling_countdown = 2500;  /* FIXME: Is 2500 calls to here == 50 seconds? */
  
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
	      sprintf(errormsg, _("Couldn't chdir() to $HOME (%s)\n"), home);
	      perror(errormsg);
	    }
	}
      else
	{
	  g_warning(_("$HOME environment variable not set\n"));
	}
      
      /* Terminate nicely so that the profiling data gets written */
      fprintf(stderr,
	      _("Bubblemon exiting.  Profiling data should be in ~/gmon.out.\n"
	      "For a good time, run 'gprof -l -p %s ~/gmon.out'.\n"),
	      (program_name?program_name:_("<name of executable>")));
      exit(EXIT_SUCCESS);
    }
#endif  /* ENABLE_PROFILING */

  if (bm->complete_redraw)
    {
      action_min = 0;
      action_max = bm->depth;
      last_action_min = 0;
      last_action_max = bm->depth;

      last_waterlevel_min = 0;
    }

  /*
    When we get here the first time we have to initialize the value
    that tells us what parts of the picture have to be updated.
  */
  if (last_action_max == -1)
    last_action_max = bm->depth;
  
  /* bm->setup is a status byte that is true if we are rolling */
  if (!bm->setup)
    return FALSE;

  /* Find out the CPU load */
  loadPercentage = get_cpu_load(bm);

  /* Find out the memory load */
  get_memory_load_percentage(bm, &memoryPercentage, &swapPercentage);

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

  waterlevel_max = 0;
  waterlevel_min = h;

  /* Move the water level with the current memory usage. */
  waterlevels_goal = h - ((memoryPercentage * h) / 100);

  /* Guard against boundary errors */
  waterlevels_goal -= 0.5;

  bm->waterlevels[0] = waterlevels_goal;
  bm->waterlevels[w - 1] = waterlevels_goal;

  for (x = 1; x < (w - 1); x++)
    {
      /* Accelerate the current waterlevel towards its correct value */
      current_waterlevel_goal = (bm->waterlevels[x - 1] +
                                 bm->waterlevels[x + 1]) / 2.0;
      bm->waterlevels_dy[x] += (current_waterlevel_goal - bm->waterlevels[x]) * VOLATILITY;
      bm->waterlevels_dy[x] *= VISCOSITY;

      if (bm->waterlevels_dy[x] > SPEED_LIMIT)
        bm->waterlevels_dy[x] = SPEED_LIMIT;
      else if (bm->waterlevels_dy[x] < -SPEED_LIMIT)
        bm->waterlevels_dy[x] = -SPEED_LIMIT;
    }

  for (x = 1; x < (w - 1); x++)
    {
      /* Move the current water level */
      bm->waterlevels[x] = bm->waterlevels[x] + bm->waterlevels_dy[x];

      if (bm->waterlevels[x] > h)
        {
          /* Stop the wave if it hits the floor... */
          bm->waterlevels[x] = h;
          bm->waterlevels_dy[x] = 0.0;
        }
      else if (bm->waterlevels[x] < 0.0)
        {
          /* ... or the ceiling. */
          bm->waterlevels[x] = 0.0;
          bm->waterlevels_dy[x] = 0.0;
        }

      /* Keep track of the highest and lowest water levels */
      if (bm->waterlevels[x] > waterlevel_max)
        waterlevel_max = bm->waterlevels[x];
      if (bm->waterlevels[x] < waterlevel_min)
        waterlevel_min = bm->waterlevels[x];
    }

  if (action_max < waterlevel_max)
    action_max = waterlevel_max;
  if (action_min > waterlevel_min)
    action_min = waterlevel_min;
  
  /*
    Vary the colors of air and water with how many
    percent of the available swap space that is in use.
  */
  watercolor = ((((NUM_COLORS / 3) - 1) * swapPercentage) / 100) * 3;
  aliascolor = watercolor + 1;
  aircolor = watercolor + 2;

  /* Sanity check the colors */
  g_assert((aircolor >= 0) && (aircolor < NUM_COLORS) &&
           (watercolor >= 0) && (watercolor < NUM_COLORS) &&
           (aliascolor >= 0) && (aliascolor < NUM_COLORS));

  /*
    Here comes the bubble magic.  Pixels are drawn by setting values in
    buf to 0-NUM_COLORS.  We should possibly make some macros or
    inline functions to {g|s}et pixels.
  */

  /*
    Draw the air-and-water background

    The waterlevel_max is the HIGHEST VALUE for the water level, which is
    actually the LOWEST VISUAL POINT of the water.  Confusing enough?

    So we want to draw from top to bottom:
      Just air from (y == 0) to (y <= waterlevel_min)
      Mixed air and water from (y == waterlevel_min) to (y <= waterlevel_max)
      Just water from (y == waterlevel_max) to (y <= h)
    
    Three loops is more code than one, but should be faster (fewer comparisons)
  */

  /* Air only */
  if (bm->complete_redraw)
    {
      /* Draw all air if a full redraw has been requested */
      for (buf_ptr = buf;
           buf_ptr < (buf + (waterlevel_min * w));
           buf_ptr++)
        *buf_ptr = aircolor;
    }
  else
    {
      for (buf_ptr = (buf + (last_waterlevel_min * w));
           buf_ptr < (buf + (waterlevel_min * w));
           buf_ptr++)
        *buf_ptr = aircolor;
    }
  last_waterlevel_min = waterlevel_min;

  /* Air and water */
  for (x = 0; x < w; x++)
    {
      /* Air... */
      for (y = waterlevel_min;
           y < ((int)(bm->waterlevels[x]));
           y++)
	buf[y * w + x] = aircolor;

      /* ... and water */
      for (; y < waterlevel_max; y++)
	buf[y * w + x] = watercolor;
    }

  /* Water only */
  for (buf_ptr = (buf + (waterlevel_max * w));
       buf_ptr < (buf + (h * w));
       buf_ptr++)
    *buf_ptr = watercolor;
  
  /* Create a new bubble if the planets are correctly aligned... */
  if ((bm->n_bubbles < MAX_BUBBLES) && ((random() % 101) <= loadPercentage))
    {
      /* We don't allow bubbles on the edges 'cause we'd have to clip them */
      bubbles[bm->n_bubbles].x = (random() % (w - 2)) + 1;
      bubbles[bm->n_bubbles].y = h - 1;
      bubbles[bm->n_bubbles].dy = 0.0;

      if (RIPPLES != 0.0)
        {
          /* Raise the water level above where the bubble is created */
          if (bubbles[bm->n_bubbles].x > 2)
            bm->waterlevels[bubbles[bm->n_bubbles].x - 2] -= RIPPLES;
          bm->waterlevels[bubbles[bm->n_bubbles].x - 1] -= RIPPLES;
          bm->waterlevels[bubbles[bm->n_bubbles].x] -= RIPPLES;
          bm->waterlevels[bubbles[bm->n_bubbles].x + 1] -= RIPPLES;
          if (bubbles[bm->n_bubbles].x < (w - 3))
            bm->waterlevels[bubbles[bm->n_bubbles].x + 2] -= RIPPLES;
        }
      
      /* Count the new bubble */
      bm->n_bubbles++;
    }
  
  /* Update and draw the bubbles */
  for (i = 0; i < bm->n_bubbles; i++)
    {
      /* Accelerate the bubble */
      bubbles[i].dy -= GRAVITY;

      /* Move the bubble vertically */
      bubbles[i].y += bubbles[i].dy;

      /* Did we lose it? */
      if (bubbles[i].y < bm->waterlevels[bubbles[i].x])
	{
          if (RIPPLES != 0.0)
            {
              /* Lower the water level around where the bubble is
                 about to vanish */
              bm->waterlevels[bubbles[i].x - 1] += RIPPLES;
              bm->waterlevels[bubbles[i].x] += 3 * RIPPLES;
              bm->waterlevels[bubbles[i].x + 1] += RIPPLES;
            }
          
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

      /*
        Keep track of which parts of the picture we have to update on
        screen.  One of the +2 is a because we don't want miss the
        lowest row of the bubble.  FIXME: the other is because of
        some kind of off-by-one problem in the following lines.
      */
      if ((y + 2) > action_max)
        action_max = y + 2;

      /* Draw the bubble in the temporary buffer */
      draw_bubble(bm, x, y, aircolor);
    }
  
  /*
    Find out which parts of the drawing are that actually needs to be
    updated
  */
  start_drawing = (last_action_min < action_min) ? last_action_min : action_min;
  stop_drawing = (last_action_max > action_max) ? last_action_max : action_max;

  /* Copy the new frame to the screen */
  update_screen(bm, start_drawing, stop_drawing);

  /* Remember where we have been poking around this round */
  last_action_max = action_max;
  last_action_min = action_min;
  
  bubblemon_set_timeout (bm);

  /* Prevent any unnecessary complete redraws */
  bm->complete_redraw = FALSE;
  
  return TRUE;
} /* bubblemon_update */


/*
 * This function, bubblemon_expose, is called whenever a portion of the
 * applet window has been exposed and needs to be redrawn.  In this
 * function, we just blit the whole pixmap onto the window.
 */
gint bubblemon_expose_handler (GtkWidget * ignored, GdkEventExpose * expose,
			       gpointer data)
{
  BubbleMonData * bm = data;

  if (!bm->setup)
    return FALSE;

  gdk_draw_image (bm->area->window, bm->area->style->fg_gc[GTK_WIDGET_STATE (bm->area)],
                  bm->image, 0, 0, 0, 0, bm->breadth, bm->depth);
  
  return FALSE; 
} /* bubblemon_expose_handler */

gint bubblemon_configure_handler (GtkWidget *widget, GdkEventConfigure *event,
				  gpointer data)
{
  BubbleMonData * bm = data;
  
  bubblemon_update ( (gpointer) bm);

  return TRUE;
}  /* bubblemon_configure_handler */

GtkWidget *applet_start_new_applet (const gchar *goad_id,
				    const char *params[],
				    int nparams)
{
  return make_new_bubblemon_applet (goad_id);
} /* applet_start_new_applet */

gint bubblemon_delete (gpointer data)
{
  BubbleMonData * bm = data;

  bm->setup = FALSE;

  if (bm->timeout)
    {
      gtk_timeout_remove (bm->timeout);
      bm->timeout = 0;
    }

  applet_widget_gtk_main_quit();

  return TRUE;  /* We do our own destruction */
}

gint bubblemon_size_change_handler(GtkWidget * w,
                                   int new_size,
                                   gpointer data)
{
  BubbleMonData * bm = data;

  /* Leave some space for the borders */
  new_size -= 4;

  /* Calculate our new dimensions */
  bm->depth = new_size;
  bm->breadth = (new_size * RELATIVE_WIDTH) / RELATIVE_HEIGHT;

  /*
    FIXME: For some unknown reason, at 16bpp, the width cannot be
    odd, or the drawing doesn't work.  I have not been able to
    determine why.  Until someone convinces me otherwise, I'll assume
    this is a bug in gdk / gtk+.  Anyway, the workaround on the next
    line kills the lowermost bit of the new width so that this bug
    never (?) gets triggered.  This is not a solution, and I hate it,
    but it's the best I can do for the moment.
  */
  bm->breadth &= ~1;

  bubblemon_set_size(bm);

  /* Redraw the whole widget after a size change */
  bm->complete_redraw = TRUE;
  
  /* FIXME: What should we return to say that everything is OK? */
  return 0;
}

/* This is the function that actually creates the display widgets */
GtkWidget *make_new_bubblemon_applet (const gchar *goad_id)
{
  BubbleMonData * bm;

  bm = g_new0 (BubbleMonData, 1);

  bm->applet = applet_widget_new (goad_id);

  if (bm->applet == NULL)
    g_error (_("Can't create applet!\n"));

  if (!glibtop_init_r (&glibtop_global_server, 0, 0))
    g_error (_("Can't open glibtop!\n"));
  
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
  bm->frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(bm->frame), GTK_SHADOW_IN);

  /* frame the drawing area */
  gtk_container_add(GTK_CONTAINER(bm->frame), bm->area);

  /* Set up the event callbacks for the area. */
  gtk_signal_connect (GTK_OBJECT (bm->area), "expose_event",
		      GTK_SIGNAL_FUNC(bubblemon_expose_handler),
		      (gpointer) bm);

  /* Add a signal to the applet for when the mouse exits to update the tooltip */
  gtk_signal_connect (GTK_OBJECT (bm->applet), "leave_notify_event",
		      GTK_SIGNAL_FUNC(widget_leave_cb),
		      (gpointer) bm);

  gtk_widget_set_events (bm->area,
			 GDK_EXPOSURE_MASK |
			 GDK_ENTER_NOTIFY_MASK);

#ifdef HAVE_CHANGE_PIXEL_SIZE
  gtk_signal_connect (GTK_OBJECT (bm->applet), "change_pixel_size",
		      GTK_SIGNAL_FUNC (bubblemon_size_change_handler),
		      (gpointer) bm);
#endif
  
  gtk_signal_connect (GTK_OBJECT (bm->applet), "save_session",
		      GTK_SIGNAL_FUNC (bubblemon_session_save),
		      (gpointer) bm);

  gtk_signal_connect (GTK_OBJECT (bm->applet), "delete_event",
                      GTK_SIGNAL_FUNC (bubblemon_delete),
		      (gpointer) bm);

  applet_widget_add (APPLET_WIDGET (bm->applet), bm->frame);

  applet_widget_register_stock_callback (APPLET_WIDGET (bm->applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _("About"),
					 about_cb,
					 bm);

  /* Initialize the CPU load metering... */
  bubblemon_setup_samples (bm);

  /* ... and our color table. */
  bubblemon_setup_colors (bm);

  /* This will prevent stuff from happening until the breadth is
     properly initialized. */
  bm->breadth = 0;

  /* Add the applet to the panel.  When we add it, we are supposed to
     receive a signal informing us about the panel width.  Thus, that
     value doesn't have to be initialized anywhere else.  Keep your
     fingers crossed :-). */
  gtk_widget_show_all (bm->applet);

#ifndef HAVE_CHANGE_PIXEL_SIZE
  /* The panel is always 48 pixels high */
  bubblemon_size_change_handler(NULL, 48, bm);
#endif

  return bm->applet;
} /* make_new_bubblemon_applet */

void bubblemon_set_timeout (BubbleMonData *bm)
{ 
  gint when = bm->update;
  
  if (when != bm->timeout_t)
    {
      if (bm->timeout)
	{
	  gtk_timeout_remove (bm->timeout);
	  bm->timeout = 0;
	}
      bm->timeout_t = when;
      bm->timeout = gtk_timeout_add (when, (GtkFunction) bubblemon_update, bm);
    }
}

void bubblemon_setup_samples (BubbleMonData *bm)
{
  /* Initialize the CPU load monitoring. */

  int i;
  u_int64_t load = 0, total = 0;

  if (bm->load)
    {
      load = bm->load[bm->loadIndex];
      free (bm->load);
    }

  if (bm->total)
    {
      total = bm->total[bm->loadIndex];
      free (bm->total);
    }

  bm->loadIndex = 0;
  bm->load = malloc (bm->samples * sizeof (u_int64_t));
  bm->total = malloc (bm->samples * sizeof (u_int64_t));
  for (i = 0; i < bm->samples; i++)
    {
      bm->load[i] = load;
      bm->total[i] = total;
    }
}

void bubblemon_setup_colors (BubbleMonData *bm)
{
  int i, j, *col;
  int r_air_noswap, g_air_noswap, b_air_noswap;
  int r_liquid_noswap, g_liquid_noswap, b_liquid_noswap;
  int r_air_maxswap, g_air_maxswap, b_air_maxswap;
  int r_liquid_maxswap, g_liquid_maxswap, b_liquid_maxswap;
  int actual_colors = NUM_COLORS / 3;

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
  
  for (i = 0; i < actual_colors; i++)
    {
      int r, g, b;
      int r_composite, g_composite, b_composite;
      char rgbStr[24];
      XColor exact, screen;

      j = i >> 1;

      /* Liquid */
      r = (r_liquid_maxswap * j + r_liquid_noswap * ((actual_colors - 1) - j)) / (actual_colors - 1);
      g = (g_liquid_maxswap * j + g_liquid_noswap * ((actual_colors - 1) - j)) / (actual_colors - 1);
      b = (b_liquid_maxswap * j + b_liquid_noswap * ((actual_colors - 1) - j)) / (actual_colors - 1);

      r_composite = r;
      g_composite = g;
      b_composite = b;

      sprintf (rgbStr, "rgb:%.2x/%.2x/%.2x", r, g, b);
      XAllocNamedColor (display, colormap, rgbStr, &exact, &screen);
      col[(i*3)] = screen.pixel;

      /* Air */
      r = (r_air_maxswap * j + r_air_noswap * ((actual_colors - 1) - j)) / (actual_colors - 1);
      g = (g_air_maxswap * j + g_air_noswap * ((actual_colors - 1) - j)) / (actual_colors - 1);
      b = (b_air_maxswap * j + b_air_noswap * ((actual_colors - 1) - j)) / (actual_colors - 1);

      r_composite += r;
      g_composite += g;
      b_composite += b;

      sprintf (rgbStr, "rgb:%.2x/%.2x/%.2x", r, g, b);
      XAllocNamedColor (display, colormap, rgbStr, &exact, &screen);
      col[(i*3) + 2] = screen.pixel;
      
      /* Anti-alias */
      r = r_composite >> 1;
      g = g_composite >> 1;
      b = b_composite >> 1;

      sprintf (rgbStr, "rgb:%.2x/%.2x/%.2x", r, g, b);
      XAllocNamedColor (display, colormap, rgbStr, &exact, &screen);
      col[(i*3) + 1] = screen.pixel;
    }
}

void
destroy_about (GtkWidget *w, gpointer data)
{
} /* destroy_about */

void about_cb (AppletWidget *widget, gpointer data)
{
  BubbleMonData *bm = data;
  char *authors[2];
  
  authors[0] = "Johan Walles <d92-jwa@nada.kth.se>";
  authors[1] = NULL;

  bm->about_box =
    gnome_about_new (_("Bubbling Load Monitor"), VERSION,
		     "Copyright (C) 1999-2000 Johan Walles",
		     (const char **) authors,
		     _("This applet displays your CPU load as a bubbling liquid.\n"
		       "This applet comes with ABSOLUTELY NO WARRANTY, "
		       "see the LICENSE file for details.\n"
		       "This is free software, and you are welcome to redistribute it "
		       "under certain conditions (GPL), "
		       "see the LICENSE file for details."),
		     NULL);

  gtk_signal_connect (GTK_OBJECT (bm->about_box), "destroy",
		      GTK_SIGNAL_FUNC (destroy_about), bm);

  gtk_widget_show (bm->about_box);
} /* about_cb */

void widget_leave_cb (GtkWidget *ignored1,
		      GdkEventAny *ignored2,
		      gpointer data)
{
  /* FIXME: This is a workaround for the gtk+ tool tip problem
     described in the TODO file. */
  update_tooltip((BubbleMonData *) data);
}

void bubblemon_set_size (BubbleMonData * bm)
{
  int i;

  if (bm->breadth == 0)
    return;
  
  gtk_widget_set_usize (bm->area, bm->breadth, bm->depth);

  /* Nuke all bubbles */
  bm->n_bubbles = 0;
  memset (bm->bubbles, 0, MAX_BUBBLES * sizeof(Bubble));
  
  /* Allocate (zeroed) bubble memory */
  if (bm->bubblebuf)
    free (bm->bubblebuf);

  bm->bubblebuf = calloc (bm->breadth * (bm->depth + 1), sizeof (int));

  /* Allocate water level memory */
  if (bm->waterlevels)
    free (bm->waterlevels);

  bm->waterlevels = malloc (bm->breadth * sizeof (float));
  for (i = 0; i < bm->breadth; i++)
    {
      bm->waterlevels[i] = bm->depth;
    }

  /* Allocate (zeroed) water level velocity memory */
  if (bm->waterlevels_dy)
    free (bm->waterlevels_dy);

  bm->waterlevels_dy = calloc (bm->breadth, sizeof (float));

  /*
    If the image has already been allocated, then free it here
    before creating a new one.
  */
  if (bm->image)
    gdk_image_destroy (bm->image);

  bm->image = gdk_image_new (GDK_IMAGE_SHARED,
                             gtk_widget_get_visual (bm->area),
                             bm->breadth,
                             bm->depth);

  if (!bm->setup)
    {
      /* Nothing is drawn until this is set. */
      bm->setup = TRUE;

      /* Schedule a timeout to get everything going */
      bubblemon_update (bm);
    }
}
