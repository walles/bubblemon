/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2004, 2008, 2009 Johan Walles - johan.walles@gmail.com
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
#include <inttypes.h>

#include <math.h>
double exp2(double x);

#include "bubblemon.h"
#include "ui.h"
#include "meter.h"
#include "mail.h"

// Bottle graphics
#include "msgInBottle.c"

void bubblemon_freeBuffers(bubblemon_t *bubblemon) {
  if (bubblemon->bubblePic.airAndWater != NULL) {
    free(bubblemon->bubblePic.airAndWater);
    bubblemon->bubblePic.airAndWater = NULL;
  }

  if (bubblemon->bubblePic.pixels != NULL) {
    free(bubblemon->bubblePic.pixels);
    bubblemon->bubblePic.pixels = NULL;
  }

  if (bubblemon->physics.waterLevels != NULL) {
    free(bubblemon->physics.waterLevels);
    bubblemon->physics.waterLevels = NULL;
  }

  if (bubblemon->physics.weeds != NULL) {
    free(bubblemon->physics.weeds);
    bubblemon->physics.weeds = NULL;
  }

  bubblemon->physics.n_bubbles = 0;
  bubblemon->physics.max_bubbles = 0;
  if (bubblemon->physics.bubbles != NULL) {
    free(bubblemon->physics.bubbles);
    bubblemon->physics.bubbles = NULL;
  }

  if (bubblemon->tooltipstring != NULL) {
    free(bubblemon->tooltipstring);
    bubblemon->tooltipstring = NULL;
  }

  if (bubblemon->createNNewBubbles != NULL) {
    free(bubblemon->createNNewBubbles);
    bubblemon->createNNewBubbles = NULL;
  }

  if (bubblemon->sortedCpuLoads != NULL) {
    free(bubblemon->sortedCpuLoads);
    bubblemon->sortedCpuLoads = NULL;
  }
}

/* Set the dimensions of the bubble array */
void bubblemon_setSize(bubblemon_t *bubblemon, int width, int height)
{
  if ((width != bubblemon->bubblePic.width) ||
      (height != bubblemon->bubblePic.height))
  {
    bubblemon_freeBuffers(bubblemon);
    bubblemon->bubblePic.width = width;
    bubblemon->bubblePic.height = height;
  }
}

static void usage2string(char *string,
			 u_int64_t used,
			 u_int64_t max)
{
  /* Create a string of the form "35/64Mb" */

  unsigned int shiftme;
  char usageString[10];
  char maxString[10];

  // We have this to be able to store either a constant string or a
  // dynamically created one in the same variable (unit).
  char *unit;
  char unitString[10];
  unit = unitString;

  if ((max >> 30) > 7000)
    {
      shiftme = 40;
      unit = _("TiB");
    }
  else if ((max >> 20) > 7000)
    {
      shiftme = 30;
      unit = _("GiB");
    }
  else if ((max >> 10) > 7000)
    {
      shiftme = 20;
      unit = _("MiB");
    }
  else if ((max >> 0) > 7000)
    {
      shiftme = 10;
      unit = _("kiB");
    }
  else
    {
      shiftme = 0;
      sprintf(unit, " %s", _("bytes"));
    }

  if ((used >> shiftme) < 10) {
    // Print usage number with one decimal digit
    double doubleUsage = used / exp2((double)shiftme);

    sprintf(usageString, "%'.1f", doubleUsage);
  } else {
    // Print integer usage
    sprintf(usageString, "%" PRIu64, used >> shiftme);
  }

  if ((max >> shiftme) < 10) {
    // Print total memory number with one decimal digit
    double doubleMax = max / exp2((double)shiftme);

    sprintf(maxString, "%'.1f", doubleMax);
  } else {
    // Print total memory as an integer
    sprintf(maxString, "%" PRIu64, max >> shiftme);
  }

  sprintf(string, "%s/%s%s",
	  usageString,
	  maxString,
	  unit);
}

const char *bubblemon_getTooltip(bubblemon_t *bubblemon)
{
  char memstring[20], swapstring[20], iowaitstring[40], loadstring[50];
  int cpu_number;

  if (!bubblemon->tooltipstring)
  {
    /* Prevent the tooltipstring buffer from overflowing on a system
       with lots of CPUs */
    bubblemon->tooltipstring =
      malloc(sizeof(char) * (bubblemon->sysload.nCpus * 50 + 200));
    assert(bubblemon->tooltipstring != NULL);
  }

  usage2string(memstring,
	       bubblemon->sysload.memoryUsed,
	       bubblemon->sysload.memorySize);
  snprintf(bubblemon->tooltipstring, 90,
           _("Memory used: %s"),
           memstring);
  if (bubblemon->sysload.swapSize > 0)
  {
    usage2string(swapstring,
		 bubblemon->sysload.swapUsed,
		 bubblemon->sysload.swapSize);
    snprintf(loadstring, 45,
	     _("\nSwap used: %s"),
	     swapstring);
    strcat(bubblemon->tooltipstring, loadstring);
  }

  snprintf(iowaitstring, 38,
	   _("\nIO wait: %d%%"),
	   bubblemon->sysload.ioLoad);
  strcat(bubblemon->tooltipstring, iowaitstring);

  if (bubblemon->sysload.nCpus == 1)
    {
      snprintf(loadstring, 45,
               _("\nCPU load: %d%%"),
               bubblemon_getCpuLoadPercentage(bubblemon, 0));
      strcat(bubblemon->tooltipstring, loadstring);
    }
  else
    {
      for (cpu_number = 0;
           cpu_number < bubblemon->sysload.nCpus;
           cpu_number++)
        {
          snprintf(loadstring, 45,
                   _("\nCPU #%d load: %d%%"),
                   cpu_number,
                   bubblemon_getCpuLoadPercentage(bubblemon, cpu_number));
          strcat(bubblemon->tooltipstring, loadstring);
        }
    }

  return bubblemon->tooltipstring;
}

/* Return the number of milliseconds that have passed since the last
   time this function was called, or -1 if this is the first call.  If
   a long time has passed since last time, the result is undefined. */
static int bubblemon_getMsecsSinceLastCall(bubblemon_t *bubblemon)
{
  int returnMe = -1;

  struct timeval currentTime;

  /* What time is it now? */
  gettimeofday(&currentTime, NULL);

  if ((bubblemon->last_sec != 0L) || (bubblemon->last_usec != 0L))
  {
    returnMe = 1000 * (int)(currentTime.tv_sec - bubblemon->last_sec);
    returnMe += ((int)(currentTime.tv_usec - bubblemon->last_usec)) / 1000L;
  }

  bubblemon->last_sec = currentTime.tv_sec;
  bubblemon->last_usec = currentTime.tv_usec;

  return returnMe;
}

static void bubblemon_updateWaterlevels(bubblemon_t *bubblemon,
					int msecsSinceLastCall)
{
  float dt = msecsSinceLastCall / 30.0;

  /* Typing fingers savers */
  int w = bubblemon->bubblePic.width;
  int h = bubblemon->bubblePic.height;

  int x;

  /* Where is the surface heading? */
  float waterLevels_goal;

  /* How high can the surface be? */
  float waterLevels_max = (h - 0.55);

  /* Move the water level with the current memory usage.  The water
   * level goes from 0.0 to h so that you can get an integer height by
   * just chopping off the decimals. */
  waterLevels_goal =
    ((float)bubblemon->sysload.memoryUsed / (float)bubblemon->sysload.memorySize) * waterLevels_max;
  if (waterLevels_goal < 3.4) {
    // If the water level is too low, the CPU load bubbles won't be
    // visible.  Enforce a minimum water level. /JW-2008dec28
    waterLevels_goal = 3.4;
  }

  bubblemon->physics.waterLevels[0].y = waterLevels_goal;
  bubblemon->physics.waterLevels[w - 1].y = waterLevels_goal;

  for (x = 1; x < (w - 1); x++)
  {
    /* Accelerate the current waterlevel towards its correct value */
    float current_waterlevel_goal = (bubblemon->physics.waterLevels[x - 1].y +
				     bubblemon->physics.waterLevels[x + 1].y) / 2.0;

    bubblemon->physics.waterLevels[x].dy +=
      (current_waterlevel_goal - bubblemon->physics.waterLevels[x].y) * dt * VOLATILITY;

    bubblemon->physics.waterLevels[x].dy *= VISCOSITY;

    if (bubblemon->physics.waterLevels[x].dy > SPEED_LIMIT)
      bubblemon->physics.waterLevels[x].dy = SPEED_LIMIT;
    else if (bubblemon->physics.waterLevels[x].dy < -SPEED_LIMIT)
      bubblemon->physics.waterLevels[x].dy = -SPEED_LIMIT;
  }

  for (x = 1; x < (w - 1); x++)
  {
    /* Move the current water level */
    bubblemon->physics.waterLevels[x].y += bubblemon->physics.waterLevels[x].dy * dt;

    if (bubblemon->physics.waterLevels[x].y > waterLevels_max)
    {
      /* Stop the wave if it hits the ceiling... */
      bubblemon->physics.waterLevels[x].y = waterLevels_max;
      bubblemon->physics.waterLevels[x].dy = 0.0;
    }
    else if (bubblemon->physics.waterLevels[x].y < 0.0)
    {
      /* ... or the floor. */
      bubblemon->physics.waterLevels[x].y = 0.0;
      bubblemon->physics.waterLevels[x].dy = 0.0;
    }
  }
}

static void bubblemon_addNourishment(bubblemon_t *bubblemon, bubblemon_Weed *weed, int percentage)
{
  float heightLimit = (bubblemon->bubblePic.height * WEED_HEIGHT) / 100.0;

  weed->nourishment += (heightLimit * percentage) / 100.0;

  if (weed->nourishment + weed->height > heightLimit)
  {
    weed->nourishment = heightLimit - weed->height;
  }
}

/* How much of the system's IO bandwidth is being used?  Returns a
 * percentage value, 0-100. */
int bubblemon_getIoLoad(bubblemon_t *bubblemon)
{
#ifdef ENABLE_PROFILING
  return 100;
#else
  return bubblemon->sysload.ioLoad;
#endif
}

static void bubblemon_updateWeeds(bubblemon_t *bubblemon, int msecsSinceLastCall)
{
  int w = bubblemon->bubblePic.width;
  int x;

  // If enough time has elapsed...
  bubblemon->timeToNextWeedsUpdate -= msecsSinceLastCall;
  while (bubblemon->timeToNextWeedsUpdate <= 0)
  {
    // ... update the nourishment level of our next weed
    int weed = random() % bubblemon->bubblePic.width;

    // Distribute the nourishment over several weeds
    int load = bubblemon_getIoLoad(bubblemon);
    if (weed > 0)
    {
      bubblemon_addNourishment(bubblemon,
			       &(bubblemon->physics.weeds[weed - 1]),
			       (load * 8) / 10);
    }
    bubblemon_addNourishment(bubblemon, &(bubblemon->physics.weeds[weed]),
			     load);
    if (weed < (bubblemon->bubblePic.width - 1))
    {
      bubblemon_addNourishment(bubblemon,
			       &(bubblemon->physics.weeds[weed + 1]),
			       (load * 8) / 10);
    }

    bubblemon->timeToNextWeedsUpdate += IOLOAD_INTERVAL;
  }

  // For all weeds...
  for (x = 0; x < w; x++)
  {
    // ... grow / shrink them according to their nourishment level
    float speed;
    float delta;

    if (bubblemon->physics.weeds[x].nourishment <= 0.0)
    {
      speed = -WEED_MINSPEED;
    }
    else
    {
      speed = bubblemon->physics.weeds[x].nourishment * WEED_SPEEDFACTOR;
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
    if (delta > bubblemon->physics.weeds[x].nourishment)
    {
      delta = bubblemon->physics.weeds[x].nourishment;
    }

    if (delta > 0.0)
    {
      bubblemon->physics.weeds[x].nourishment -= delta;
    }
    bubblemon->physics.weeds[x].height += delta;
    if (bubblemon->physics.weeds[x].height < 0.0)
    {
      bubblemon->physics.weeds[x].height = 0.0;
    }
  }
}

/* Create a new bubble at the given X coordinate.
 *
 * X must be > 0 and < bubblePic.width - 2.  The reason we don't allow
 * 0 or bubblePic.width - 1 is that then we'd have to do clipping.
 * Clipping is boring.
 */
static void bubblemon_createBubble(bubblemon_t *bubblemon,
				   int x, bubblemon_layer_t layer)
{
  /* We don't allow bubbles on the edges 'cause we'd have to clip them */
  assert(x >= 1);
  assert(x <= bubblemon->bubblePic.width - 2);

  if (bubblemon->physics.n_bubbles < bubblemon->physics.max_bubbles)
  {
    bubblemon->physics.bubbles[bubblemon->physics.n_bubbles].x = x;
    bubblemon->physics.bubbles[bubblemon->physics.n_bubbles].y = 0.0;
    bubblemon->physics.bubbles[bubblemon->physics.n_bubbles].dy = 0.0;
    /* Create alternately foreground and background bubbles */
    bubblemon->physics.bubbles[bubblemon->physics.n_bubbles].layer = layer;

    if (RIPPLES != 0.0)
    {
      bubblemon_WaterLevel *waterLevels = bubblemon->physics.waterLevels;
      bubblemon_Bubble *bubbles = bubblemon->physics.bubbles;

      /* Raise the water level above where the bubble is created */
      if ((bubbles[bubblemon->physics.n_bubbles].x - 2) >= 0)
	waterLevels[bubbles[bubblemon->physics.n_bubbles].x - 2].y += RIPPLES;
      waterLevels[bubbles[bubblemon->physics.n_bubbles].x - 1].y   += RIPPLES;
      waterLevels[bubbles[bubblemon->physics.n_bubbles].x].y       += RIPPLES;
      waterLevels[bubbles[bubblemon->physics.n_bubbles].x + 1].y   += RIPPLES;
      if ((bubbles[bubblemon->physics.n_bubbles].x + 2) < bubblemon->bubblePic.width)
	waterLevels[bubbles[bubblemon->physics.n_bubbles].x + 2].y += RIPPLES;
    }

    /* Count the new bubble */
    bubblemon->physics.n_bubbles++;
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
static void bubblemon_createBubbles(bubblemon_t *bubblemon, int msecsSinceLastCall)
{
  if (bubblemon->createNNewBubbles == NULL) {
    bubblemon->createNNewBubbles = calloc(bubblemon->sysload.nCpus, sizeof(float));
    assert(bubblemon->createNNewBubbles != NULL);
  }

  if (bubblemon->sortedCpuLoads == NULL) {
    bubblemon->sortedCpuLoads = calloc(bubblemon->sysload.nCpus, sizeof(int));
    assert(bubblemon->sortedCpuLoads != NULL);
  }

  // Sort the CPU loads.  The point is to reflect the highest loaded
  // CPU in the middle and the less loaded ones further out towards
  // the edges.
  int cpu;
  for (cpu = 0; cpu < bubblemon->sysload.nCpus; cpu++) {
    bubblemon->sortedCpuLoads[cpu] = bubblemon_getCpuLoadPercentage(bubblemon, cpu);
  }
  qsort(bubblemon->sortedCpuLoads,
	bubblemon->sysload.nCpus,
	sizeof(*bubblemon->sortedCpuLoads),
	intCompare);

  float dt = msecsSinceLastCall / 30.0;

  // We have this many pixels for bubble rendering.  Don't allow
  // bubbles on the edges because clipping is boring.
  float allCandidatePixels = bubblemon->bubblePic.width - 2;
  float perCpuCandidatePixels = allCandidatePixels / (float)bubblemon->sysload.nCpus;

  // For each CPU...
  for (cpu = 0; cpu < bubblemon->sysload.nCpus; cpu++) {
    // ... should we create any bubbles?
    bubblemon->createNNewBubbles[cpu] += ((float)bubblemon->sortedCpuLoads[cpu] *
			       perCpuCandidatePixels *
			       dt) / 2000.0;

    while (bubblemon->createNNewBubbles[cpu] >= 1.0) {
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

      bubblemon_layer_t layer =
	random() % 120 < bubblemon->sortedCpuLoads[cpu] ? FOREGROUND : BACKGROUND;
      // Add one pixel to avoid having to clip at the left edge
      bubblemon_createBubble(bubblemon, x + 1.0, layer);

      // Count the new bubble
      bubblemon->createNNewBubbles[cpu]--;
    }
  }
}

/* Update the bubbles (and possibly create new ones) */
static void bubblemon_updateBubbles(bubblemon_t *bubblemon, int msecsSinceLastCall)
{
  bubblemon_createBubbles(bubblemon, msecsSinceLastCall);

  /* Move the bubbles */
  int i;
  float dt = msecsSinceLastCall / 30.0;
  for (i = 0; i < bubblemon->physics.n_bubbles; i++)
  {
    /* Accelerate the bubble upwards */
    bubblemon->physics.bubbles[i].dy -= GRAVITY * dt;

    /* Move the bubble vertically */
    bubblemon->physics.bubbles[i].y +=
      bubblemon->physics.bubbles[i].dy * dt *
      ((bubblemon->physics.bubbles[i].layer == BACKGROUND) ? BGBUBBLE_SPEED : 1.0);

    /* Did we lose it? */
    if (bubblemon->physics.bubbles[i].y > bubblemon->physics.waterLevels[bubblemon->physics.bubbles[i].x].y)
    {
      if (RIPPLES != 0.0)
      {
        /* Lower the water level around where the bubble is
           about to vanish */
        bubblemon->physics.waterLevels[bubblemon->physics.bubbles[i].x - 1].y -= RIPPLES;
        bubblemon->physics.waterLevels[bubblemon->physics.bubbles[i].x].y     -= 3 * RIPPLES;
        bubblemon->physics.waterLevels[bubblemon->physics.bubbles[i].x + 1].y -= RIPPLES;
      }

      /* We just lost it, so let's nuke it */
      bubblemon->physics.bubbles[i].x  = bubblemon->physics.bubbles[bubblemon->physics.n_bubbles - 1].x;
      bubblemon->physics.bubbles[i].y  = bubblemon->physics.bubbles[bubblemon->physics.n_bubbles - 1].y;
      bubblemon->physics.bubbles[i].dy = bubblemon->physics.bubbles[bubblemon->physics.n_bubbles - 1].dy;
      bubblemon->physics.n_bubbles--;

      /* We must check the previously last bubble, which is
        now the current bubble, also. */
      i--;
      continue;
    }
  }
}

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/* Update the bottle */
static void bubblemon_updateBottle(bubblemon_t *bubblemon,
				   int msecsSinceLastCall,
				   mail_status_t mailStatus)
{
  float dt = msecsSinceLastCall / 30.0;
  float gravityDdy;
  float dragDdy;
  int isInWater;

  // The MIN stuff prevents the bottle from floating above the visible
  // area even when the water surface is at the top of the display
  isInWater =
    (bubblemon->physics.bottle_y
     < MIN(bubblemon->physics.waterLevels[bubblemon->bubblePic.width / 2].y,
	   bubblemon->bubblePic.height - msgInBottle.height / 2));

  if ((mailStatus != NO_MAIL) && (bubblemon->physics.bottle_state == GONE)) {
    /* Mail is available but the bottle's gone.  Drop a new bottle */
    bubblemon->physics.bottle_y = bubblemon->bubblePic.height + msgInBottle.height;
    bubblemon->physics.bottle_dy = 0.0;
    bubblemon->physics.bottle_state = FALLING;
  } else if (mailStatus == UNREAD_MAIL) {
    /* Float the bottle as long as there's unread mail. */
    bubblemon->physics.bottle_state = FLOATING;
  } else if ((mailStatus == NO_MAIL) && (bubblemon->physics.bottle_state == GONE)) {
    /* No mail, no bottle, everybody's happy */
  } else if ((mailStatus == READ_MAIL) && (bubblemon->physics.bottle_state == SUNK)) {
    /* Only read mail left and the bottle has sunk, everybody's happy */
  } else {
    /* The bottle is there, but there's no unread mail.  Sink the
     * bottle. */
    bubblemon->physics.bottle_state = SINKING;
  }

  if (bubblemon->physics.bottle_state == GONE) {
    return;
  }

  /* Accelerate the bottle */
  if (bubblemon->physics.bottle_state == SUNK) {
    gravityDdy = 0;
  } else if ((bubblemon->physics.bottle_state == FALLING) ||
      (bubblemon->physics.bottle_state == SINKING) ||
      !isInWater)
  {
    // Gravity pulls the bottle down
    gravityDdy = 2.0 * GRAVITY * dt;
  } else {
    // Gravity floats the bottle up
    gravityDdy = 2.0 * -(GRAVITY * dt);
  }

  if (isInWater) {
    dragDdy = (bubblemon->physics.bottle_dy * bubblemon->physics.bottle_dy) * BOTTLE_DRAG;
    // If the speed is positive...
    if (bubblemon->physics.bottle_dy > 0.0) {
      // ... then the drag should be negative
      dragDdy = -dragDdy;
    }
  } else {
    dragDdy = 0.0;
  }

  bubblemon->physics.bottle_dy += gravityDdy + dragDdy;

  /* Move the bottle vertically */
  bubblemon->physics.bottle_y += bubblemon->physics.bottle_dy * dt;

  // If the bottle has fallen on screen...
  if ((bubblemon->physics.bottle_state == FALLING) &&
      (bubblemon->physics.bottle_y < bubblemon->bubblePic.height))
  {
    // ... it should start floating instead of falling
    bubblemon->physics.bottle_state = FLOATING;
  }

  /* Did it hit bottom? */
  if (bubblemon->physics.bottle_state == SINKING) {
    if ((mailStatus == READ_MAIL)
	&& (bubblemon->physics.bottle_y <= msgInBottle.height / 4.0)) {
      /* Keep it there */
      bubblemon->physics.bottle_y = msgInBottle.height / 4.0;
      bubblemon->physics.bottle_state = SUNK;
      bubblemon->physics.bottle_dy = 0.0;
    } else if (bubblemon->physics.bottle_y + msgInBottle.height < 0.0) {
      /* Remove it */
      bubblemon->physics.bottle_state = GONE;
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

  if (amount == 0) {
     return c1;
  }

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

  /*
  assert((amount != 0)   || (returnme.value == c1.value));
  assert((amount != 255) || (returnme.value == c2.value));
  */

  return returnme;
}

/* Update the bubble array from the system load */
static void bubblemon_updatePhysics(bubblemon_t *bubblemon,
				    int msecsSinceLastCall,
				    mail_status_t mailStatus)
{
  /* No size -- no go */
  if ((bubblemon->bubblePic.width == 0) ||
      (bubblemon->bubblePic.height == 0))
  {
    assert(bubblemon->bubblePic.pixels == NULL);
    assert(bubblemon->bubblePic.airAndWater == NULL);
    return;
  }

  /* Make sure the water levels exist */
  if (bubblemon->physics.waterLevels == NULL)
  {
    bubblemon->physics.waterLevels =
      (bubblemon_WaterLevel *)calloc(bubblemon->bubblePic.width, sizeof(bubblemon_WaterLevel));
    assert(bubblemon->physics.waterLevels != NULL);
  }

  /* Make sure the sea-weeds exist */
  if (bubblemon->physics.weeds == NULL)
  {
    int i;

    bubblemon->physics.weeds =
      (bubblemon_Weed *)calloc(bubblemon->bubblePic.width, sizeof(bubblemon_Weed));
    assert(bubblemon->physics.weeds != NULL);

    // Colorize the weeds
    for (i = 0; i < bubblemon->bubblePic.width; i++)
    {
      bubblemon->physics.weeds[i].color =
	bubblemon_interpolateColor(bubblemon_constant2color(WEEDCOLOR0),
				   bubblemon_constant2color(WEEDCOLOR1),
				   (random() % 156) + bubblemon->stripey);
      bubblemon->stripey = 100 - bubblemon->stripey;
    }
  }

  /* Make sure the bubbles exist */
  if (bubblemon->physics.bubbles == NULL)
  {
    bubblemon->physics.n_bubbles = 0;
    // FIXME: max_bubbles should be the width * the time it takes for
    // a bubble to rise all the way to the ceiling.
    bubblemon->physics.max_bubbles = (bubblemon->bubblePic.width * bubblemon->bubblePic.height) / 5;

    bubblemon->physics.bubbles =
      (bubblemon_Bubble *)calloc(bubblemon->physics.max_bubbles, sizeof(bubblemon_Bubble));
    assert(bubblemon->physics.bubbles != NULL);
  }

  /* Update our universe */
  bubblemon_updateWaterlevels(bubblemon, msecsSinceLastCall);
  bubblemon_updateWeeds(bubblemon, msecsSinceLastCall);
  bubblemon_updateBubbles(bubblemon, msecsSinceLastCall);
  bubblemon_updateBottle(bubblemon, msecsSinceLastCall, mailStatus);
}

int bubblemon_getMemoryPercentage(bubblemon_t *bubblemon)
{
  int returnme;

  if (bubblemon->sysload.memorySize == 0L)
  {
    returnme = 0;
  }
  else
  {
    returnme = (int)((200L * bubblemon->sysload.memoryUsed + 1) / (2 * bubblemon->sysload.memorySize));
  }

#ifdef ENABLE_PROFILING
  return 100;
#else
  return returnme;
#endif
}

int bubblemon_getSwapPercentage(bubblemon_t *bubblemon)
{
  int returnme;

  if (bubblemon->sysload.swapSize == 0L)
  {
    returnme = 0;
  }
  else
  {
    returnme = (int)((200L * bubblemon->sysload.swapUsed + 1) / (2 * bubblemon->sysload.swapSize));
  }

#ifdef ENABLE_PROFILING
  return 100;
#else
  return returnme;
#endif
}

/* The cpu parameter is the cpu number, 0 - #CPUs-1. */
int bubblemon_getCpuLoadPercentage(bubblemon_t *bubblemon, int cpuNo)
{
  assert(cpuNo >= 0 && cpuNo < bubblemon->sysload.nCpus);

#ifdef ENABLE_PROFILING
  return 100;
#else
  return bubblemon->sysload.cpuLoad[cpuNo];
#endif
}

int bubblemon_getAverageLoadPercentage(bubblemon_t *bubblemon)
{
  int cpuNo;
  int totalLoad;

  totalLoad = 0;
  for (cpuNo = 0; cpuNo < bubblemon->sysload.nCpus; cpuNo++)
  {
    totalLoad += bubblemon_getCpuLoadPercentage(bubblemon, cpuNo);
  }
  totalLoad = totalLoad / bubblemon->sysload.nCpus;

  assert(totalLoad >= 0);
  assert(totalLoad <= 100);

#ifdef ENABLE_PROFILING
  return 100;
#else
  return totalLoad;
#endif
}

static void bubblemon_censorLoad(bubblemon_t *bubblemon)
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

  assert(bubblemon->sysload.memorySize > 0);

  censoredMemUsed = bubblemon->sysload.swapUsed + bubblemon->sysload.memoryUsed;

  if (censoredMemUsed > bubblemon->sysload.memorySize)
    {
      censoredSwapUsed = censoredMemUsed - bubblemon->sysload.memorySize;
      censoredMemUsed = bubblemon->sysload.memorySize;
    }
  else
    {
      censoredSwapUsed = 0;
    }

  /* Sanity check that we don't use more swap/mem than what's available */
  assert((censoredMemUsed <= bubblemon->sysload.memorySize) &&
         (censoredSwapUsed <= bubblemon->sysload.swapSize));

  bubblemon->sysload.memoryUsed = censoredMemUsed;
  bubblemon->sysload.swapUsed = censoredSwapUsed;

#ifdef ENABLE_PROFILING
  bubblemon->sysload.memoryUsed = bubblemon->sysload.memorySize;
  bubblemon->sysload.swapUsed = bubblemon->sysload.swapSize;
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

// Render bubbles, waterlevels and stuff into the bubblePic
static void bubblemon_environmentToBubbleArray(bubblemon_t *bubblemon,
					       bubblemon_layer_t layer)
{
  bubblemon_picture_t *bubblePic = &bubblemon->bubblePic;
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

    float waterHeight = bubblemon->physics.waterLevels[x].y;
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
  bubble = bubblemon->physics.bubbles;
  hf = h; // Move the int->float cast out of the loop
  for (i = 0; i < bubblemon->physics.n_bubbles; i++)
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

static void bubblemon_bubbleArrayToPixmap(bubblemon_t *bubblemon,
					  bubblemon_layer_t layer)
{
  bubblemon_picture_t *bubblePic = &bubblemon->bubblePic;

  bubblemon_color_t noSwapAirColor;
  bubblemon_color_t noSwapWaterColor;

  bubblemon_color_t maxSwapAirColor;
  bubblemon_color_t maxSwapWaterColor;

  bubblemon_color_t colors[3];

  bubblemon_colorcode_t *airOrWater;
  bubblemon_color_t *pixel;
  int i;

  int w, h;

  noSwapAirColor = bubblemon_constant2color(NOSWAPAIRCOLOR);
  noSwapWaterColor = bubblemon_constant2color(NOSWAPWATERCOLOR);

  maxSwapAirColor = bubblemon_constant2color(MAXSWAPAIRCOLOR);
  maxSwapWaterColor = bubblemon_constant2color(MAXSWAPWATERCOLOR);

  colors[AIR] = bubblemon_interpolateColor(noSwapAirColor,
					   maxSwapAirColor,
					   (bubblemon_getSwapPercentage(bubblemon) * 255) / 100);
  colors[WATER] = bubblemon_interpolateColor(noSwapWaterColor,
					     maxSwapWaterColor,
					     (bubblemon_getSwapPercentage(bubblemon) * 255) / 100);
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
      bubblemon_color_t currentColor = *pixel;
      bubblemon_color_t newColor = colors[*airOrWater++];
      if (newColor.value != currentColor.value) {
        *pixel = bubblemon_interpolateColor(currentColor,
                                            newColor,
                                            newColor.components.a);
      }
      pixel++;
    }
  }
}

static void bubblemon_bottleToPixmap(bubblemon_t *bubblemon)
{
  bubblemon_picture_t *bubblePic = &bubblemon->bubblePic;
  int bottleX, bottleY;
  int bottleW, bottleH;
  int bottleYMin, bottleYMax;
  int pictureX0, pictureY0;
  int pictureW, pictureH;

  if (bubblemon->physics.bottle_state == GONE) {
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
  pictureY0 = pictureH - bubblemon->physics.bottle_y - bottleH / 2;

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

static void bubblemon_weedsToPixmap(bubblemon_t *bubblemon)
{
  bubblemon_picture_t *bubblePic = &bubblemon->bubblePic;
  int w = bubblePic->width;
  int h = bubblePic->height;

  int x;

  for (x = 0; x < w; x++)
  {
    int y;
    int highestWeedPixel = h - bubblemon->physics.weeds[x].height;

    for (y = highestWeedPixel; y < h; y++)
    {
      bubblePic->pixels[y * w + x] = bubblemon_interpolateColor(bubblePic->pixels[y * w + x],
								bubblemon->physics.weeds[x].color,
								bubblemon->physics.weeds[x].color.components.a);
    }
  }
}

const bubblemon_picture_t *bubblemon_getPicture(bubblemon_t *bubblemon)
{
  static const int msecsPerPhysicsFrame = 1000 / PHYSICS_FRAMERATE;

  int msecsSinceLastCall;
  mail_status_t mailStatus = mail_getMailStatus();

  // Make sure we never try to move things backwards
  do {
    msecsSinceLastCall = bubblemon_getMsecsSinceLastCall(bubblemon);
  } while (msecsSinceLastCall < 0);

  // Get the system load
  meter_getLoad(&bubblemon->sysload);
  bubblemon_censorLoad(bubblemon);

  // If a "long" time has passed since the last frame, settle for
  // updating the physics just a bit of the way.
  if (msecsSinceLastCall > 200)
  {
    msecsSinceLastCall = 200;
  }

  // Push the universe around
  if (msecsSinceLastCall <= msecsPerPhysicsFrame)
  {
    bubblemon_updatePhysics(bubblemon, msecsSinceLastCall, mailStatus);
  }
  else
  {
    while (bubblemon->physicalTimeElapsed < msecsSinceLastCall)
    {
      bubblemon_updatePhysics(bubblemon, msecsPerPhysicsFrame, mailStatus);
      bubblemon->physicalTimeElapsed += msecsPerPhysicsFrame;
    }
    bubblemon->physicalTimeElapsed -= msecsSinceLastCall;
  }

  // Draw the pixels
  bubblemon_environmentToBubbleArray(bubblemon, BACKGROUND);
  bubblemon_bubbleArrayToPixmap(bubblemon, BACKGROUND);

  bubblemon_weedsToPixmap(bubblemon);
  bubblemon_bottleToPixmap(bubblemon);

  bubblemon_environmentToBubbleArray(bubblemon, FOREGROUND);
  bubblemon_bubbleArrayToPixmap(bubblemon, FOREGROUND);

  return &bubblemon->bubblePic;
}

bubblemon_t *bubblemon_init(void)
{
  bubblemon_t *bubblemon;

#ifdef ENABLE_PROFILING
  fprintf(stderr,
	  "Warning: " PACKAGE " has been configured with --enable-profiling and will show max\n"
	  "load all the time.\n");
#endif

  bubblemon = calloc(1, sizeof(bubblemon_t));
  assert(bubblemon != NULL);

  // Initialize the random number generation
  srandom(time(NULL));

  // Initialize the load metering
  meter_init(&bubblemon->sysload);

  // Initialize the bottle
  bubblemon->physics.bottle_state = GONE;

  return bubblemon;
}

void bubblemon_done(bubblemon_t *bubblemon)
{
  // Terminate the load metering
  meter_done(&bubblemon->sysload);

  // Free our data holding structure
  bubblemon_freeBuffers(bubblemon);
  free(bubblemon);
}
