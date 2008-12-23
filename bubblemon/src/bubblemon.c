/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2004, 2008 Johan Walles - johan.walles@gmail.com
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

/*
 * This is a platform independent file that drives the program.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>

#include <math.h>
double exp2(double x);

#include "bubblemon.h"
#include "ui.h"
#include "meter.h"
#include "mail.h"
#include "netload.h"

// Bottle graphics
#include "msgInBottle.c"

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

    if (physics.weeds != NULL)
    {
      free(physics.weeds);
      physics.weeds = NULL;
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

static void usage2string(/*@out@*/ char *string,
			 u_int64_t used,
			 u_int64_t max)
{
  /* Create a string of the form "35/64Mb" */

  unsigned int shiftme = 0;
  char divisor_char = '\0';
  const char *formatstring;

  if ((max >> 30) > 7000)
    {
      shiftme = 40;
      divisor_char = 'T';
    }
  else if ((max >> 20) > 7000)
    {
      shiftme = 30;
      divisor_char = 'G';
    }
  else if ((max >> 10) > 7000)
    {
      shiftme = 20;
      divisor_char = 'M';
    }
  else if ((max >> 0) > 7000)
    {
      shiftme = 10;
      divisor_char = 'k';
    }

  if ((used >> shiftme) < 10) {
    // Print usage number with one decimal digit
    double doubleUsage = used / exp2((double)shiftme);

    if (divisor_char == '\0') {
      formatstring = _("%.1f/%llu bytes");
    } else {
      formatstring = _("%.1f/%llu%cb");
    }

    sprintf(string, formatstring,
	    doubleUsage,
	    max >> shiftme,
	    divisor_char);
  } else {
    // Print integer usage
    if (divisor_char == '\0') {
      formatstring = _("%llu/%llu bytes");
    } else {
      formatstring = _("%llu/%llu%cb");
    }

    sprintf(string, formatstring,
	    used >> shiftme,
	    max >> shiftme,
	    divisor_char);
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
    assert(tooltipstring != NULL);
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

  /* How high can the surface be? */
  float waterLevels_max = (h - 0.55);

  /* Move the water level with the current memory usage.  The water
   * level goes from 0.0 to h so that you can get an integer height by
   * just chopping off the decimals. */
  waterLevels_goal =
    ((float)sysload.memoryUsed / (float)sysload.memorySize) * waterLevels_max;

  physics.waterLevels[0].y = waterLevels_goal;
  physics.waterLevels[w - 1].y = waterLevels_goal;

  for (x = 1; x < (w - 1); x++)
  {
    /* Accelerate the current waterlevel towards its correct value */
    float current_waterlevel_goal = (physics.waterLevels[x - 1].y +
				     physics.waterLevels[x + 1].y) / 2.0;

    physics.waterLevels[x].dy +=
      (current_waterlevel_goal - physics.waterLevels[x].y) * dt * VOLATILITY;

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

    if (physics.waterLevels[x].y > waterLevels_max)
    {
      /* Stop the wave if it hits the ceiling... */
      physics.waterLevels[x].y = waterLevels_max;
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

static void bubblemon_addNourishment(bubblemon_Weed *weed, int percentage)
{
  float heightLimit = (bubblePic.height * WEED_HEIGHT) / 100.0;

  weed->nourishment += (heightLimit * percentage) / 100.0;

  if (weed->nourishment + weed->height > heightLimit)
  {
    weed->nourishment = heightLimit - weed->height;
  }
}

static void bubblemon_updateWeeds(int msecsSinceLastCall)
{
  static int timeToNextUpdate = 0;

  int w = bubblePic.width;
  int x;

  // If enough time has elapsed...
  timeToNextUpdate -= msecsSinceLastCall;
  while (timeToNextUpdate <= 0)
  {
    // ... update the nourishment level of our next weed
    int weed = random() % bubblePic.width;

    // Distribute the nourishment over several weeds
    if (weed > 0)
    {
      bubblemon_addNourishment(&(physics.weeds[weed - 1]), (netload_getLoadPercentage() * 8) / 10);
    }
    bubblemon_addNourishment(&(physics.weeds[weed]), netload_getLoadPercentage());
    if (weed < (bubblePic.width - 1))
    {
      bubblemon_addNourishment(&(physics.weeds[weed + 1]), (netload_getLoadPercentage() * 8) / 10);
    }

    timeToNextUpdate += NETLOAD_INTERVAL;
  }

  // For all weeds...
  for (x = 0; x < w; x++)
  {
    // ... grow / shrink them according to their nourishment level
    float speed;
    float delta;

    if (physics.weeds[x].nourishment <= 0.0)
    {
      speed = -WEED_MINSPEED;
    }
    else
    {
      speed = physics.weeds[x].nourishment * WEED_SPEEDFACTOR;
      if (speed > WEED_MAXSPEED)
      {
	speed = WEED_MAXSPEED;
      }
      else if (speed < WEED_MINSPEED)
      {
	speed = WEED_MINSPEED;
      }
    }

    delta = (speed * msecsSinceLastCall) / 1000.0;
    if (delta > physics.weeds[x].nourishment)
    {
      delta = physics.weeds[x].nourishment;
    }

    if (delta > 0.0)
    {
      physics.weeds[x].nourishment -= delta;
    }
    physics.weeds[x].height += delta;
    if (physics.weeds[x].height < 0.0)
    {
      physics.weeds[x].height = 0.0;
    }
  }
}

/* Create a new bubble at the given X coordinate.
 *
 * X must be > 0 and < bubblePic.width - 2.  The reason we don't allow
 * 0 or bubblePic.width - 1 is that then we'd have to do clipping.
 * Clipping is boring.
 */
static void bubblemon_createBubble(int x, bubblemon_layer_t layer)
{
  /* We don't allow bubbles on the edges 'cause we'd have to clip them */
  assert(x >= 1);
  assert(x <= bubblePic.width - 2);

  if (physics.n_bubbles < physics.max_bubbles)
  {
    physics.bubbles[physics.n_bubbles].x = x;
    physics.bubbles[physics.n_bubbles].y = 0.0;
    physics.bubbles[physics.n_bubbles].dy = 0.0;
    /* Create alternately foreground and background bubbles */
    physics.bubbles[physics.n_bubbles].layer = layer;

    if (RIPPLES != 0.0)
    {
      /* Raise the water level above where the bubble is created */
      if ((physics.bubbles[physics.n_bubbles].x - 2) >= 0)
	physics.waterLevels[physics.bubbles[physics.n_bubbles].x - 2].y += RIPPLES;
      physics.waterLevels[physics.bubbles[physics.n_bubbles].x - 1].y   += RIPPLES;
      physics.waterLevels[physics.bubbles[physics.n_bubbles].x].y       += RIPPLES;
      physics.waterLevels[physics.bubbles[physics.n_bubbles].x + 1].y   += RIPPLES;
      if ((physics.bubbles[physics.n_bubbles].x + 2) < bubblePic.width)
	physics.waterLevels[physics.bubbles[physics.n_bubbles].x + 2].y += RIPPLES;
    }

    /* Count the new bubble */
    physics.n_bubbles++;
  }
}

/* Compare two ints.  Used by bubblemon_createBubbles().  See the
 * qsort(3) man page for info. */
static int intCompare(const void *ap, const void *bp) {
  int a = *(int*)ap;
  int b = *(int*)bp;

  if (a < b) {
    return -1;
  } else if (a > b) {
    return 1;
  } else {
    return 0;
  }
}

/* Create new bubbles as needed. */
static void bubblemon_createBubbles(int msecsSinceLastCall)
{
  static float *createNNewBubbles;
  if (createNNewBubbles == NULL) {
    createNNewBubbles = calloc(sysload.nCpus, sizeof(float));
    assert(createNNewBubbles != NULL);
  }

  static int *sortedCpuLoads;
  if (sortedCpuLoads == NULL) {
    sortedCpuLoads = calloc(sysload.nCpus, sizeof(int));
    assert(sortedCpuLoads != NULL);
  }

  // Sort the CPU loads.  The point is to reflect the highest loaded
  // CPU in the middle and the less loaded ones further out towards
  // the edges.
  int cpu;
  for (cpu = 0; cpu < sysload.nCpus; cpu++) {
    sortedCpuLoads[cpu] = bubblemon_getCpuLoadPercentage(cpu);
  }
  qsort(sortedCpuLoads, sysload.nCpus, sizeof(*sortedCpuLoads), intCompare);

  float dt = msecsSinceLastCall / 30.0;

  // We have this many pixels for bubble rendering.  Don't allow
  // bubbles on the edges because clipping is boring.
  float allCandidatePixels = bubblePic.width - 2;
  float perCpuCandidatePixels = allCandidatePixels / (float)sysload.nCpus;

  // For each CPU...
  for (cpu = 0; cpu < sysload.nCpus; cpu++) {
    // ... should we create any bubbles?
    createNNewBubbles[cpu] += ((float)sortedCpuLoads[cpu] *
			       perCpuCandidatePixels *
			       dt) / 2000.0;

    while (createNNewBubbles[cpu] >= 1.0) {
      // Yes!  Bubbles needed.

      float x = (random() % (long int)(perCpuCandidatePixels * 1000.0)) / 1000.0;
      if (x < perCpuCandidatePixels / 2) {
	// Put the new bubble on the left half of the display
	x += (perCpuCandidatePixels / 2.0) * (float)cpu;
      } else {
	// Put the new bubble on the right half of the display
	x -= perCpuCandidatePixels / 2.0;

	// Assume we have 100 pixels to draw on.  In that case, pixel
	// number 99 is the highest we can draw on.
	float globalHighestOkPixel = allCandidatePixels - 1.0;
	float thisHighestOkPixel = globalHighestOkPixel - (perCpuCandidatePixels / 2.0) * (float)cpu;
	x = thisHighestOkPixel - x;
      }

      // Add one pixel to avoid having to clip at the left edge
      bubblemon_layer_t layer =
	random() % 120 < sortedCpuLoads[cpu] ? FOREGROUND : BACKGROUND;
      bubblemon_createBubble(x + 1.0, layer);

      // Count the new bubble
      createNNewBubbles[cpu]--;
    }
  }
}

/* Update the bubbles (and possibly create new ones) */
static void bubblemon_updateBubbles(int msecsSinceLastCall)
{
  bubblemon_createBubbles(msecsSinceLastCall);

  /* Move the bubbles */
  int i;
  float dt = msecsSinceLastCall / 30.0;
  for (i = 0; i < physics.n_bubbles; i++)
  {
    /* Accelerate the bubble upwards */
    physics.bubbles[i].dy -= GRAVITY * dt;

    /* Move the bubble vertically */
    physics.bubbles[i].y +=
      physics.bubbles[i].dy * dt *
      ((physics.bubbles[i].layer == BACKGROUND) ? BGBUBBLE_SPEED : 1.0);

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

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/* Update the bottle */
static void bubblemon_updateBottle(int msecsSinceLastCall,
				   mail_status_t mailStatus)
{
  float dt = msecsSinceLastCall / 30.0;
  float gravityDdy;
  float dragDdy;
  int isInWater;

  // The MIN stuff prevents the bottle from floating above the visible
  // area even when the water surface is at the top of the display
  isInWater =
    physics.bottle_y < MIN(physics.waterLevels[bubblePic.width / 2].y,
			   bubblePic.height - msgInBottle.height / 2);

  if ((mailStatus != NO_MAIL) && (physics.bottle_state == GONE)) {
    /* Mail is available but the bottle's gone.  Drop a new bottle */
    physics.bottle_y = bubblePic.height + msgInBottle.height;
    physics.bottle_dy = 0.0;
    physics.bottle_state = FALLING;
  } else if (mailStatus == UNREAD_MAIL) {
    /* Float the bottle as long as there's unread mail. */
    physics.bottle_state = FLOATING;
  } else if ((mailStatus == NO_MAIL) && (physics.bottle_state == GONE)) {
    /* No mail, no bottle, everybody's happy */
  } else if ((mailStatus == READ_MAIL) && (physics.bottle_state == SUNK)) {
    /* Only read mail left and the bottle has sunk, everybody's happy */
  } else {
    /* The bottle is there, but there's no unread mail.  Sink the
     * bottle. */
    physics.bottle_state = SINKING;
  }

  if (physics.bottle_state == GONE) {
    return;
  }

  /* Accelerate the bottle */
  if (physics.bottle_state == SUNK) {
    gravityDdy = 0;
  } else if ((physics.bottle_state == FALLING) ||
      (physics.bottle_state == SINKING) ||
      !isInWater)
  {
    // Gravity pulls the bottle down
    gravityDdy = 2.0 * GRAVITY * dt;
  } else {
    // Gravity floats the bottle up
    gravityDdy = 2.0 * -(GRAVITY * dt);
  }

  if (isInWater) {
    dragDdy = (physics.bottle_dy * physics.bottle_dy) * BOTTLE_DRAG;
    // If the speed is positive...
    if (physics.bottle_dy > 0.0) {
      // ... then the drag should be negative
      dragDdy = -dragDdy;
    }
  } else {
    dragDdy = 0.0;
  }

  physics.bottle_dy += gravityDdy + dragDdy;

  /* Move the bottle vertically */
  physics.bottle_y += physics.bottle_dy * dt;

  // If the bottle has fallen on screen...
  if ((physics.bottle_state == FALLING) &&
      (physics.bottle_y < bubblePic.height))
  {
    // ... it should start floating instead of falling
    physics.bottle_state = FLOATING;
  }

  /* Did it hit bottom? */
  if (physics.bottle_state == SINKING) {
    if ((mailStatus == READ_MAIL)
	&& (physics.bottle_y <= msgInBottle.height / 4.0)) {
      /* Keep it there */
      physics.bottle_y = msgInBottle.height / 4.0;
      physics.bottle_state = SUNK;
      physics.bottle_dy = 0.0;
    } else if (physics.bottle_y + msgInBottle.height < 0.0) {
      /* Remove it */
      physics.bottle_state = GONE;
    }
  }
}


static bubblemon_color_t bubblemon_constant2color(const unsigned int constant)
{
  bubblemon_color_t returnMe;

  returnMe.components.r = (constant >> 24) & 0xff;
  returnMe.components.g = (constant >> 16) & 0xff;
  returnMe.components.b = (constant >> 8)  & 0xff;
  returnMe.components.a = (constant >> 0)  & 0xff;

  return returnMe;
}

/* The amount parameter is 0-255. */
static inline bubblemon_color_t bubblemon_interpolateColor(const bubblemon_color_t c1,
							   const bubblemon_color_t c2,
							   const int amount)
{
  int a, r, g, b;
  bubblemon_color_t returnme;

  assert(amount >= 0 && amount < 256);

  r = ((((int)c1.components.r) * (255 - amount)) +
       (((int)c2.components.r) * amount)) / 255;
  g = ((((int)c1.components.g) * (255 - amount)) +
       (((int)c2.components.g) * amount)) / 255;
  b = ((((int)c1.components.b) * (255 - amount)) +
       (((int)c2.components.b) * amount)) / 255;
  a = ((((int)c1.components.a) * (255 - amount)) +
       (((int)c2.components.a) * amount)) / 255;

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

  returnme.components.r = r;
  returnme.components.g = g;
  returnme.components.b = b;
  returnme.components.a = a;

  assert((amount != 0)   || (returnme.value == c1.value));
  assert((amount != 255) || (returnme.value == c2.value));

  return returnme;
}

/* Update the bubble array from the system load */
static void bubblemon_updatePhysics(int msecsSinceLastCall,
				    mail_status_t mailStatus)
{
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
      (bubblemon_WaterLevel *)calloc(bubblePic.width, sizeof(bubblemon_WaterLevel));
    assert(physics.waterLevels != NULL);
  }

  /* Make sure the sea-weeds exist */
  if (physics.weeds == NULL)
  {
    int i;

    physics.weeds =
      (bubblemon_Weed *)calloc(bubblePic.width, sizeof(bubblemon_Weed));
    assert(physics.weeds != NULL);

    // Colorize the weeds
    for (i = 0; i < bubblePic.width; i++)
    {
      static int stripey = 0;

      physics.weeds[i].color =
	bubblemon_interpolateColor(bubblemon_constant2color(WEEDCOLOR0),
				   bubblemon_constant2color(WEEDCOLOR1),
				   (random() % 156) + stripey);
      stripey = 100 - stripey;
    }
  }

  /* Make sure the bubbles exist */
  if (physics.bubbles == NULL)
  {
    physics.n_bubbles = 0;
    // FIXME: max_bubbles should be the width * the time it takes for
    // a bubble to rise all the way to the ceiling.
    physics.max_bubbles = (bubblePic.width * bubblePic.height) / 5;

    physics.bubbles =
      (bubblemon_Bubble *)calloc(physics.max_bubbles, sizeof(bubblemon_Bubble));
    assert(physics.bubbles != NULL);
  }

  /* Update our universe */
  bubblemon_updateWaterlevels(msecsSinceLastCall);
  bubblemon_updateWeeds(msecsSinceLastCall);
  bubblemon_updateBubbles(msecsSinceLastCall);
  bubblemon_updateBottle(msecsSinceLastCall, mailStatus);
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

/* The cpu parameter is the cpu number, 0 - #CPUs-1. */
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

#ifdef ENABLE_PROFILING
  sysload.memoryUsed = sysload.memorySize;
  sysload.swapUsed = sysload.swapSize;
#endif
}

static void bubblemon_draw_bubble(/*@out@*/ bubblemon_picture_t *bubblePic,
				  int x,
				  int y)
{
  bubblemon_colorcode_t *bubbleBuf = bubblePic->airAndWater;
  int w = bubblePic->width;
  int h = bubblePic->height;
  bubblemon_colorcode_t *buf_ptr;

  /*
    Clipping is not necessary for x, but it is for y.
    To prevent ugliness, we draw aliascolor only on top of
    watercolor, and aircolor on top of aliascolor.
  */

  /* Top row */
  buf_ptr = &(bubbleBuf[(y - 1) * w + x - 1]);
  if (y > 0)
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

static void bubblemon_environmentToBubbleArray(bubblemon_picture_t *bubblePic,
					       bubblemon_layer_t layer)
{
  // Render bubbles, waterlevels and stuff into the bubblePic
  int w = bubblePic->width;
  int h = bubblePic->height;
  int x, y, i;
  float hf;

  bubblemon_Bubble *bubble;

  if (bubblePic->airAndWater == NULL)
  {
    bubblePic->airAndWater = (bubblemon_colorcode_t *)malloc(w * h * sizeof(bubblemon_colorcode_t));
    assert(bubblePic->airAndWater != NULL);
  }

  // Draw the air and water background
  for (x = 0; x < w; x++)
  {
    bubblemon_colorcode_t *pixel;
    double dummy;

    float waterHeight = physics.waterLevels[x].y;
    int nAirPixels = h - waterHeight;
    int antialias = 0;

    if (modf(waterHeight, &dummy) >= 0.5) {
      nAirPixels--;
      antialias = 1;
    }

    pixel = &(bubblePic->airAndWater[x]);
    for (y = 0; y < nAirPixels; y++)
    {
      *pixel = AIR;
      pixel += w;
    }

    if (antialias) {
      *pixel = ANTIALIAS;
      pixel += w;
      y++;
    }

    for (; y < h; y++) {
      *pixel = WATER;
      pixel += w;
    }
  }

  // Draw the bubbles
  bubble = physics.bubbles;
  hf = h; // Move the int->float cast out of the loop
  for (i = 0; i < physics.n_bubbles; i++)
  {
    if (bubble->layer >= layer)
    {
      bubblemon_draw_bubble(bubblePic,
			    bubble->x,
			    hf - bubble->y);
    }
    bubble++;
  }
}

static void bubblemon_bubbleArrayToPixmap(bubblemon_picture_t *bubblePic,
					  bubblemon_layer_t layer)
{
  static bubblemon_color_t noSwapAirColor;
  static bubblemon_color_t noSwapWaterColor;

  static bubblemon_color_t maxSwapAirColor;
  static bubblemon_color_t maxSwapWaterColor;

  bubblemon_color_t colors[3];

  bubblemon_colorcode_t *airOrWater;
  bubblemon_color_t *pixel;
  int i;

  int w, h;

  noSwapAirColor = bubblemon_constant2color(NOSWAPAIRCOLOR);
  noSwapWaterColor = bubblemon_constant2color(NOSWAPWATERCOLOR);

  maxSwapAirColor = bubblemon_constant2color(MAXSWAPAIRCOLOR);
  maxSwapWaterColor = bubblemon_constant2color(MAXSWAPWATERCOLOR);

  colors[AIR] = bubblemon_interpolateColor(noSwapAirColor, maxSwapAirColor, (bubblemon_getSwapPercentage() * 255) / 100);
  colors[WATER] = bubblemon_interpolateColor(noSwapWaterColor, maxSwapWaterColor, (bubblemon_getSwapPercentage() * 255) / 100);
  colors[ANTIALIAS] = bubblemon_interpolateColor(colors[AIR], colors[WATER], 128);

  w = bubblePic->width;
  h = bubblePic->height;

  /* Make sure the pixel array exist */
  if (bubblePic->pixels == NULL)
  {
    bubblePic->pixels =
      (bubblemon_color_t *)malloc(bubblePic->width * bubblePic->height * sizeof(bubblemon_color_t));
    assert(bubblePic->pixels != NULL);
  }

  pixel = bubblePic->pixels;
  airOrWater = bubblePic->airAndWater;
  if (layer == BACKGROUND) {
    // Erase the old pic as we draw
    for (i = 0; i < w * h; i++) {
      *pixel++ = colors[*airOrWater++];
    }
  } else {
    // Draw on top of the current pic
    for (i = 0; i < w * h; i++) {
      bubblemon_color_t myColor = colors[*airOrWater++];
      *pixel = bubblemon_interpolateColor(*pixel,
					  myColor,
					  myColor.components.a);
      pixel++;
    }
  }
}

static void bubblemon_bottleToPixmap(bubblemon_picture_t *bubblePic)
{
  int bottleX, bottleY;
  int bottleW, bottleH;
  int bottleYMin, bottleYMax;
  int pictureX0, pictureY0;
  int pictureW, pictureH;

  if (physics.bottle_state == GONE) {
    return;
  }

  assert(bubblePic->pixels != NULL);

  pictureW = bubblePic->width;
  pictureH = bubblePic->height;

  bottleW = msgInBottle.width;
  bottleH = msgInBottle.height;

  // Ditch the bottle if the image is too small
  // FIXME: Should we check the image height as well?
  if (pictureW < (bottleW + 4)) {
    return;
  }

  // Center the bottle horizontally
  pictureX0 = (bubblePic->width - bottleW) / 2;
  // Position the bottle vertically
  pictureY0 = pictureH - physics.bottle_y - bottleH / 2;

  // Clip the bottle vertically to fit it into the picture
  bottleYMin = 0;
  if (pictureY0 < 0) {
    bottleYMin = -pictureY0;
  }
  bottleYMax = bottleH;
  if (pictureY0 + bottleH >= pictureH) {
    bottleYMax = bottleH - (pictureY0 + bottleH - pictureH);
  }

  // Iterate over the bottle
  for (bottleX = 0; bottleX < bottleW; bottleX++) {
    for (bottleY = bottleYMin; bottleY < bottleYMax; bottleY++) {
      int pictureX = pictureX0 + bottleX;
      int pictureY = pictureY0 + bottleY;

      bubblemon_color_t bottlePixel;
      bubblemon_color_t *picturePixel;

      // assert(pictureY < pictureH);

      bottlePixel = ((bubblemon_color_t*)(msgInBottle.pixel_data))[bottleX + bottleY * bottleW];
      picturePixel = &(bubblePic->pixels[pictureX + pictureY * pictureW]);

      *picturePixel = bubblemon_interpolateColor(*picturePixel,
						 bottlePixel,
						 bottlePixel.components.a);
    }
  }
}

static void bubblemon_weedsToPixmap(bubblemon_picture_t *bubblePic)
{
  int w = bubblePic->width;
  int h = bubblePic->height;

  int x;

  for (x = 0; x < w; x++)
  {
    int y;
    int highestWeedPixel = h - physics.weeds[x].height;

    for (y = highestWeedPixel; y < h; y++)
    {
      bubblePic->pixels[y * w + x] = bubblemon_interpolateColor(bubblePic->pixels[y * w + x],
								physics.weeds[x].color,
								physics.weeds[x].color.components.a);
    }
  }
}

const bubblemon_picture_t *bubblemon_getPicture()
{
  static const int msecsPerPhysicsFrame = 1000 / PHYSICS_FRAMERATE;
  static int physicalTimeElapsed = 0;

  int msecsSinceLastCall;
  mail_status_t mailStatus = mail_getMailStatus();

  // Make sure we never try to move things backwards
  do {
    msecsSinceLastCall = bubblemon_getMsecsSinceLastCall();
  } while (msecsSinceLastCall < 0);

  // Get the system load
  meter_getLoad(&sysload);
  bubblemon_censorLoad();

  // Update the network load statistics.  Since they measure a real
  // time phenomenon, they need to keep track of how much real time
  // has actually passed since last time.  Thus, we provide the
  // uncensored msecsSinceLastCall.
  netload_updateLoadstats(msecsSinceLastCall);

  // If a "long" time has passed since the last frame, settle for
  // updating the physics just a bit of the way.
  if (msecsSinceLastCall > 200)
  {
    msecsSinceLastCall = 200;
  }

  // Push the universe around
  if (msecsSinceLastCall <= msecsPerPhysicsFrame)
  {
    bubblemon_updatePhysics(msecsSinceLastCall, mailStatus);
  }
  else
  {
    while (physicalTimeElapsed < msecsSinceLastCall)
    {
      bubblemon_updatePhysics(msecsPerPhysicsFrame, mailStatus);
      physicalTimeElapsed += msecsPerPhysicsFrame;
    }
    physicalTimeElapsed -= msecsSinceLastCall;
  }

  // Draw the pixels
  bubblemon_environmentToBubbleArray(&bubblePic, BACKGROUND);
  bubblemon_bubbleArrayToPixmap(&bubblePic, BACKGROUND);

  bubblemon_weedsToPixmap(&bubblePic);
  bubblemon_bottleToPixmap(&bubblePic);

  bubblemon_environmentToBubbleArray(&bubblePic, FOREGROUND);
  bubblemon_bubbleArrayToPixmap(&bubblePic, FOREGROUND);

  return &bubblePic;
}

void bubblemon_init(void)
{
#ifdef ENABLE_PROFILING
  fprintf(stderr,
	  "Warning: " PACKAGE " has been configured with --enable-profiling and will show max\n"
	  "load all the time.\n");
#endif

  // Initialize the random number generation
  srandom(time(NULL));

  // Initialize the load metering
  meter_init(&sysload);
  sysload.cpuLoad = (int *)calloc(sysload.nCpus, sizeof(int));
  assert(sysload.cpuLoad != NULL);

  // Initialize the bottle
  physics.bottle_state = GONE;
}

void bubblemon_done(void)
{
  // Terminate the load metering
  meter_done();
}
