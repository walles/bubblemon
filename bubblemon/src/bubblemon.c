/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2000 Johan Walles - d92-jwa@nada.kth.se
 *  http://www.nada.kth.se/~d92-jwa/code/#bubblemon
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

/*
 * This is a platform independent file that drives the program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>

/* Natural language translation stuff */
#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>

#ifndef _
#define _ gettext
#endif

#else

#define _

#endif // ENABLE_NLS

#include "bubblemon.h"
#include "ui.h"
#include "meter.h"
#include "config.h"

static bubblemon_picture_t bubblePic;
static bubblemon_Physics physics;
static meter_sysload_t sysload;

/* Set the dimensions of the bubble array */
void bubblemon_setSize(int width, int height)
{
  if ((width != bubblePic.width) ||
      (height != bubblePic.height))
  {
    bubblePic.width = width;
    bubblePic.height = height;

    if (bubblePic.airAndWater != NULL)
    {
      free(bubblePic.airAndWater);
      bubblePic.airAndWater = NULL;
    }

    if (bubblePic.pixels != NULL)
    {
      free(bubblePic.pixels);
      bubblePic.pixels = NULL;
    }

    if (physics.waterLevels != NULL)
    {
      free(physics.waterLevels);
      physics.waterLevels = NULL;
    }

    physics.n_bubbles = 0;
    physics.max_bubbles = 0;
    if (physics.bubbles != NULL)
    {
      free(physics.bubbles);
      physics.bubbles = NULL;
    }
  }
}

static void usage2string(char *string,
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
      sprintf(string, "%lld/%lld%cb",
	      used >> shiftme,
	      max >> shiftme,
	      divisor_char);
    }
  else
    {
      sprintf(string, "%lld/%lld bytes",
	      used >> shiftme,
	      max >> shiftme);
    }
}

const char *bubblemon_getTooltip(void)
{
  char memstring[20], swapstring[20], loadstring[50];
  static char *tooltipstring = 0;
  int cpu_number;

  if (!tooltipstring)
  {
    /* Prevent the tooltipstring buffer from overflowing on a system
       with lots of CPUs */
    tooltipstring =
      malloc(sizeof(char) * (sysload.nCpus * 50 + 100));
  }

  usage2string(memstring, sysload.memoryUsed, sysload.memorySize);
  snprintf(tooltipstring, 90,
           _("Memory used: %s"),
           memstring);
  if (sysload.swapSize > 0)
  {
    usage2string(swapstring, sysload.swapUsed, sysload.swapSize);
    snprintf(loadstring, 90,
	     _("\nSwap used: %s"),
	     swapstring);
    strcat(tooltipstring, loadstring);
  }

  if (sysload.nCpus == 1)
    {
      snprintf(loadstring, 45,
               _("\nCPU load: %d%%"),
               bubblemon_getCpuLoadPercentage(0));
      strcat(tooltipstring, loadstring);
    }
  else
    {
      for (cpu_number = 0;
           cpu_number < sysload.nCpus;
           cpu_number++)
        {
          snprintf(loadstring, 45,
                   _("\nCPU #%d load: %d%%"),
                   cpu_number,
                   bubblemon_getCpuLoadPercentage(cpu_number));
          strcat(tooltipstring, loadstring);
        }
    }
  
  return tooltipstring;
}

static int bubblemon_getMsecsSinceLastCall()
{
  /* Return the number of milliseconds that have passed since the last
     time this function was called, or -1 if this is the first call.  If
     a long time has passed since last time, the result is undefined. */
  static long last_sec = 0L;
  static long last_usec = 0L;

  int returnMe = -1;

  struct timeval currentTime;

  /* What time is it now? */
  gettimeofday(&currentTime, NULL);

  if ((last_sec != 0L) || (last_usec != 0L))
  {
    returnMe = 1000 * (int)(currentTime.tv_sec - last_sec);
    returnMe += ((int)(currentTime.tv_usec - last_usec)) / 1000L;
  }

  last_sec = currentTime.tv_sec;
  last_usec = currentTime.tv_usec;

  return returnMe;
}

static void bubblemon_updateWaterlevels(int msecsSinceLastCall)
{
  float dt = msecsSinceLastCall / 30.0;

  /* Typing fingers savers */
  int w = bubblePic.width;
  int h = bubblePic.height;
  
  int x;
  
  /* Where is the surface heading? */
  float waterLevels_goal;
  
  /* Move the water level with the current memory usage. */
  waterLevels_goal = ((bubblemon_getMemoryPercentage() * h) / 100);

  /* Guard against boundary errors */
  waterLevels_goal += 0.5;

  physics.waterLevels[0].y = waterLevels_goal;
  physics.waterLevels[w - 1].y = waterLevels_goal;

  for (x = 1; x < (w - 1); x++)
  {
    /* Accelerate the current waterlevel towards its correct value */
    float current_waterlevel_goal = (physics.waterLevels[x - 1].y +
                                     physics.waterLevels[x + 1].y) / 2.0;
    physics.waterLevels[x].dy += (current_waterlevel_goal - physics.waterLevels[x].y) * VOLATILITY;
    physics.waterLevels[x].dy *= VISCOSITY;
    
    if (physics.waterLevels[x].dy > SPEED_LIMIT)
      physics.waterLevels[x].dy = SPEED_LIMIT;
    else if (physics.waterLevels[x].dy < -SPEED_LIMIT)
      physics.waterLevels[x].dy = -SPEED_LIMIT;
  }
  
  for (x = 1; x < (w - 1); x++)
  {
    /* Move the current water level */
    physics.waterLevels[x].y += physics.waterLevels[x].dy * dt;
    
    if (physics.waterLevels[x].y > h)
    {
      /* Stop the wave if it hits the ceiling... */
      physics.waterLevels[x].y = h;
      physics.waterLevels[x].dy = 0.0;
    }
    else if (physics.waterLevels[x].y < 0.0)
    {
      /* ... or the floor. */
      physics.waterLevels[x].y = 0.0;
      physics.waterLevels[x].dy = 0.0;
    }
  }
}

/* Update the bubbles (and possibly create new ones) */
static void bubblemon_updateBubbles(int msecsSinceLastCall)
{
  int i;
  int createNNewBubbles;

  float dt = msecsSinceLastCall / 30.0;

  /* Typing fingers saver */
  int w = bubblePic.width;
  
  /* Create new bubble(s) if the planets are correctly aligned... */
  createNNewBubbles = ((float)(random() % 11) *
		       (float)bubblemon_getAverageLoadPercentage() *
		       (float)bubblePic.width *
		       msecsSinceLastCall) / 300000.0 + 0.5;
  
  for (i = 0; i < createNNewBubbles; i++)
  {
    if (physics.n_bubbles < physics.max_bubbles)
    {
      /* We don't allow bubbles on the edges 'cause we'd have to clip them */
      physics.bubbles[physics.n_bubbles].x = (random() % (w - 2)) + 1;
      physics.bubbles[physics.n_bubbles].y = 0.0;
      physics.bubbles[physics.n_bubbles].dy = 0.0;
      
      if (RIPPLES != 0.0)
      {
	/* Raise the water level above where the bubble is created */
	if ((physics.bubbles[physics.n_bubbles].x - 2) >= 0)
	  physics.waterLevels[physics.bubbles[physics.n_bubbles].x - 2].y += RIPPLES;
	physics.waterLevels[physics.bubbles[physics.n_bubbles].x - 1].y   += RIPPLES;
	physics.waterLevels[physics.bubbles[physics.n_bubbles].x].y       += RIPPLES;
	physics.waterLevels[physics.bubbles[physics.n_bubbles].x + 1].y   += RIPPLES;
	if ((physics.bubbles[physics.n_bubbles].x + 2) < w)
	  physics.waterLevels[physics.bubbles[physics.n_bubbles].x + 2].y += RIPPLES;
      }
    
      /* Count the new bubble */
      physics.n_bubbles++;
    }
  }
  
  /* Move the bubbles */
  for (i = 0; i < physics.n_bubbles; i++)
  {
    /* Accelerate the bubble upwards */
    physics.bubbles[i].dy -= GRAVITY * dt;
    
    /* Move the bubble vertically */
    physics.bubbles[i].y += physics.bubbles[i].dy * dt;
    
    /* Did we lose it? */
    if (physics.bubbles[i].y > physics.waterLevels[physics.bubbles[i].x].y)
    {
      if (RIPPLES != 0.0)
      {
        /* Lower the water level around where the bubble is
           about to vanish */
        physics.waterLevels[physics.bubbles[i].x - 1].y -= RIPPLES;
        physics.waterLevels[physics.bubbles[i].x].y     -= 3 * RIPPLES;
        physics.waterLevels[physics.bubbles[i].x + 1].y -= RIPPLES;
      }
      
      /* We just lost it, so let's nuke it */
      physics.bubbles[i].x  = physics.bubbles[physics.n_bubbles - 1].x;
      physics.bubbles[i].y  = physics.bubbles[physics.n_bubbles - 1].y;
      physics.bubbles[i].dy = physics.bubbles[physics.n_bubbles - 1].dy;
      physics.n_bubbles--;
      
      /* We must check the previously last bubble, which is
        now the current bubble, also. */
      i--;
      continue;
    }
  }
}

/* Update the bubble array from the system load */
static void bubblemon_updatePhysics()
{
  int msecsSinceLastCall = bubblemon_getMsecsSinceLastCall();
  
  /* No size -- no go */
  if ((bubblePic.width == 0) ||
      (bubblePic.height == 0))
  {
    assert(bubblePic.pixels == NULL);
    assert(bubblePic.airAndWater == NULL);
    return;
  }
  
  /* Make sure the water levels exist */
  if (physics.waterLevels == NULL)
  {
    physics.waterLevels =
      (bubblemon_WaterLevel *)malloc(bubblePic.width * sizeof(bubblemon_WaterLevel));
  }

  /* Make sure the bubbles exist */
  if (physics.bubbles == NULL)
  {
    physics.n_bubbles = 0;
    // FIXME: max_bubbles should be the width * the time it takes for
    // a bubble to rise all the way to the ceiling.
    physics.max_bubbles = (bubblePic.width * bubblePic.height) / 5;
    
    physics.bubbles =
      (bubblemon_Bubble *)malloc(physics.max_bubbles * sizeof(bubblemon_Bubble));
  }
  
  /* Update our universe */
  bubblemon_updateWaterlevels(msecsSinceLastCall);
  bubblemon_updateBubbles(msecsSinceLastCall);
}

int bubblemon_getMemoryPercentage()
{
  int returnme;
  
  if (sysload.memorySize == 0L)
  {
    returnme = 0;
  }
  else
  {
    returnme = (int)((200L * sysload.memoryUsed + 1) / (2 * sysload.memorySize));
  }

#ifdef ENABLE_PROFILING
  return 100;
#else
  return returnme;
#endif
}

int bubblemon_getSwapPercentage()
{
  int returnme;
  
  if (sysload.swapSize == 0L)
  {
    returnme = 0;
  }
  else
  {
    returnme = (int)((200L * sysload.swapUsed + 1) / (2 * sysload.swapSize));
  }

#ifdef ENABLE_PROFILING
  return 100;
#else
  return returnme;
#endif
}

/* The cpu parameter is the cpu number, 1 - #CPUs.  0 means average load */
int bubblemon_getCpuLoadPercentage(int cpuNo)
{
  assert(cpuNo >= 0 && cpuNo < sysload.nCpus);
  
#ifdef ENABLE_PROFILING
  return 100;
#else
  return sysload.cpuLoad[cpuNo];
#endif
}

int bubblemon_getAverageLoadPercentage()
{
  int cpuNo;
  int totalLoad;
  
  totalLoad = 0;
  for (cpuNo = 0; cpuNo < sysload.nCpus; cpuNo++)
  {
    totalLoad += bubblemon_getCpuLoadPercentage(cpuNo);
  }
  totalLoad = totalLoad / sysload.nCpus;
  
  assert(totalLoad >= 0);
  assert(totalLoad <= 100);

#ifdef ENABLE_PROFILING
  return 100;
#else
  return totalLoad;
#endif
}

static void bubblemon_censorLoad()
{
  /* Calculate the projected memory + swap load to show the user.  The
     values given to the user is how much memory the system is using
     relative to the total amount of electronic RAM.  The swap color
     indicates how much memory above the amount of electronic RAM the
     system is using.
    
     This scheme does *not* show how the system has decided to
     allocate swap and electronic RAM to the users' processes.
  */

  u_int64_t censoredMemUsed;
  u_int64_t censoredSwapUsed;

  assert(sysload.memorySize > 0);

  censoredMemUsed = sysload.swapUsed + sysload.memoryUsed;

  if (censoredMemUsed > sysload.memorySize)
    {
      censoredSwapUsed = censoredMemUsed - sysload.memorySize;
      censoredMemUsed = sysload.memorySize;
    }
  else
    {
      censoredSwapUsed = 0;
    }

  /* Sanity check that we don't use more swap/mem than what's available */
  assert((censoredMemUsed <= sysload.memorySize) &&
         (censoredSwapUsed <= sysload.swapSize));

  sysload.memoryUsed = censoredMemUsed;
  sysload.swapUsed = censoredSwapUsed;
}

static void bubblemon_draw_bubble(bubblemon_picture_t *bubblePic,
				  int x,
				  int y)
{
  bubblemon_colorcode_t *bubbleBuf = bubblePic->airAndWater;
  int w = bubblePic->width;
  int h = bubblePic->height;
  bubblemon_colorcode_t *buf_ptr;
  
  /*
    Clipping is not necessary for x, but it *is* for y.
    To prevent ugliness, we draw aliascolor only on top of
    watercolor, and aircolor on top of aliascolor.
  */
  
  /* Top row */
  buf_ptr = &(bubbleBuf[(y - 1) * w + x - 1]);
  if (y > physics.waterLevels[x].y)
  {
    if (*buf_ptr != AIR)
    {
      (*buf_ptr)++ ;
    }
    buf_ptr++;
    
    *buf_ptr = AIR; buf_ptr++;
    
    if (*buf_ptr != AIR)
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
  *buf_ptr = AIR; buf_ptr++;
  *buf_ptr = AIR; buf_ptr++;
  *buf_ptr = AIR; buf_ptr += (w - 2);
  
  /* Bottom row */
  if (y < (h - 1))
  {
    if (*buf_ptr != AIR)
    {
      (*buf_ptr)++ ;
    }
    buf_ptr++;
    
    *buf_ptr = AIR; buf_ptr++;
    
    if (*buf_ptr != AIR)
    {
      (*buf_ptr)++ ;
    }
  }
}

static void bubblemon_physicsToBubbleArray(bubblemon_picture_t *bubblePic)
{
  // Render bubbles, waterlevels and stuff into the bubblePic
  int w = bubblePic->width;
  int h = bubblePic->height;
  int x, y, i;

  bubblemon_Bubble *bubble;

  if (bubblePic->airAndWater == NULL)
  {
    bubblePic->airAndWater = (bubblemon_colorcode_t *)malloc(w * h * sizeof(bubblemon_colorcode_t));
  }
  
  // Draw the air and water background
  for (x = 0; x < w; x++)
  {
    bubblemon_colorcode_t *pixel;
    bubblemon_colorcode_t *oldpixel;

    pixel = &(bubblePic->airAndWater[x]);
    for (y = 0; y < (h - physics.waterLevels[x].y); y++)
    {
      *pixel = AIR;
      pixel += w;
    }

    y = (h - physics.waterLevels[x].y);
    oldpixel = pixel;
    pixel = &(bubblePic->airAndWater[(y + 0) * w + x]);
    /*
    if (x == 0)
    {
      printf("Difference is %d pixels, a line is %d pixels.\n",
	     (((unsigned int)pixel) - ((unsigned int)oldpixel)) / sizeof(pixel),
	     w);
      assert(pixel == oldpixel);
    }
    */
    for (y = (h - physics.waterLevels[x].y); y < h; y++)
    {
      *pixel = WATER;
      pixel += w;
    }
  }
  
  // Draw the bubbles
  bubble = physics.bubbles;
  for (i = 0; i < physics.n_bubbles; i++)
  {
    bubblemon_draw_bubble(bubblePic,
			  bubble->x,
			  h - bubble->y);
    bubble++;
  }
}

static bubblemon_color_t bubblemon_interpolateColor(const bubblemon_color_t c1,
						    const bubblemon_color_t c2,
						    const int percentage)
{
  int a, r, g, b;
  bubblemon_color_t returnme;

  assert(percentage >= 0 && percentage <= 100);

  a = ((((int)c1.components.a) * (100 - percentage)) +
       (((int)c2.components.a) * percentage)) / 100;
  r = ((((int)c1.components.r) * (100 - percentage)) +
       (((int)c2.components.r) * percentage)) / 100;
  g = ((((int)c1.components.g) * (100 - percentage)) +
       (((int)c2.components.g) * percentage)) / 100;
  b = ((((int)c1.components.b) * (100 - percentage)) +
       (((int)c2.components.b) * percentage)) / 100;

  /*
  assert(a >= 0 && a <= 255);
  assert(r >= 0 && r <= 255);
  assert(g >= 0 && g <= 255);
  assert(b >= 0 && b <= 255);

  assert(((c1.components.a <= a) && (a <= c2.components.a)) ||
	 ((c1.components.a >= a) && (a >= c2.components.a)));
  assert(((c1.components.r <= r) && (r <= c2.components.r)) ||
	 ((c1.components.r >= r) && (r >= c2.components.r)));
  assert(((c1.components.g <= g) && (g <= c2.components.g)) ||
	 ((c1.components.g >= g) && (g >= c2.components.g)));
  assert(((c1.components.b <= b) && (b <= c2.components.b)) ||
	 ((c1.components.b >= b) && (b >= c2.components.b)));
  */
  
  returnme.components.a = a;
  returnme.components.r = r;
  returnme.components.g = g;
  returnme.components.b = b;

  assert((percentage != 0)   || (returnme.value == c1.value));
  assert((percentage != 100) || (returnme.value == c2.value));

  return returnme;
}

static void bubblemon_bubbleArrayToPixmap(bubblemon_picture_t *bubblePic)
{
  bubblemon_color_t colors[3];

  bubblemon_colorcode_t *airOrWater;
  bubblemon_color_t *pixel;
  int i;
  
  int w, h;

  colors[AIR] = bubblemon_interpolateColor(NOSWAPAIRCOLOR, MAXSWAPAIRCOLOR, bubblemon_getSwapPercentage());
  colors[WATER] = bubblemon_interpolateColor(NOSWAPWATERCOLOR, MAXSWAPWATERCOLOR, bubblemon_getSwapPercentage());
  colors[ANTIALIAS] = bubblemon_interpolateColor(colors[AIR], colors[WATER], 50);

  w = bubblePic->width;
  h = bubblePic->height;

  /* Make sure the pixel array exist */
  if (bubblePic->pixels == NULL)
  {
    bubblePic->pixels =
      (bubblemon_color_t *)malloc(bubblePic->width * bubblePic->height * sizeof(bubblemon_color_t));
  }

  pixel = bubblePic->pixels;
  airOrWater = bubblePic->airAndWater;
  for (i = 0; i < w * h; i++)
  {
    *pixel++ = colors[*airOrWater++];
  }
}

const bubblemon_picture_t *bubblemon_getPicture()
{
  // Get the system load
  meter_getLoad(&sysload);
  bubblemon_censorLoad();
  
  // Push the universe around
  bubblemon_updatePhysics();
  
  // Convert the system load into a water-and-air array
  bubblemon_physicsToBubbleArray(&bubblePic);

  // Convert the water-and-air array into a colormap
  bubblemon_bubbleArrayToPixmap(&bubblePic);
  
  return &bubblePic;
}

int main(int argc, char *argv[])
{
  int exitcode;

#ifdef ENABLE_PROFILING
  g_warning(PACKAGE "has been configured with --enable-profiling and will show max\n"
	    "load all the time.\n");
#endif
  
  // Initialize the load metering
  meter_init(argc, argv, &sysload);
  sysload.cpuLoad = (int *)malloc(sizeof(int) * sysload.nCpus);

  // Do the disco duck
  exitcode = ui_main(argc, argv);
  
  // Terminate the load metering
  meter_done();

  return exitcode;
}
