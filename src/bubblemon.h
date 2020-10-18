/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2004, 2009, 2010 Johan Walles - johan.walles@gmail.com
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

#ifndef BUBBLEMON_H
#define BUBBLEMON_H

#ifndef __APPLE__
#include <config.h>
#endif
#include <sys/types.h>

#ifndef _
// GNU Gettext not available
#define _(X) X
#endif

#include "meter.h"

/* How fast do the bubbles rise? */
#define GRAVITY -0.01

/* The drag coefficient of the bottle */
#define BOTTLE_DRAG 0.1

/* How fast do the water levels accelerate? */
#define VOLATILITY 1.0

/* 0.0 means the liquid never moves.
   1.0 means the liquid will continue to oscillate forever. */
#define VISCOSITY 0.99

/* How fast are the water levels allowed to move? */
#define SPEED_LIMIT 1.0

/* How much newly created bubbles make the surface ripple */
#define RIPPLES 0.2

/* The applet's dimensions */
#define RELATIVE_WIDTH 32
#define RELATIVE_HEIGHT 40

/* How fast background bubbles move relative to the foreground
 * bubbles. */
#define BGBUBBLE_SPEED 0.4

/* How fast the weed grows in pixels/sec */
#define WEED_MAXSPEED 10
#define WEED_MINSPEED 3
#define WEED_SPEEDFACTOR 1

/* Swap usage color scale                                 rrggbbaa */
static const unsigned int NOSWAPAIRCOLOR    = (unsigned)0x75ceff00;
static const unsigned int NOSWAPWATERCOLOR  = (unsigned)0x0066ff80;

static const unsigned int MAXSWAPAIRCOLOR   = (unsigned)0xff333340;
static const unsigned int MAXSWAPWATERCOLOR = (unsigned)0xaa000080;

/* Weeds have a random color between these two */
static const unsigned int WEEDCOLOR0        = (unsigned)0x00ff0080;
static const unsigned int WEEDCOLOR1        = (unsigned)0xffff40ff;

/* How many times per sec the physics get updated */
#define PHYSICS_FRAMERATE 100

/* How often is the IO load meter updated?  The unit is milliseconds
 * between updates. */
#define IOLOAD_INTERVAL 250

/* The maximum height of the weeds indicating the IO load, expressed
 * in percent of the full height */
#define WEED_HEIGHT 40

/* Color code constants */
typedef enum { WATER, ANTIALIAS, AIR } bubblemon_colorcode_t;

/* Bottle behaviour */
typedef enum { GONE, FLOATING, SINKING, FALLING, SUNK } bubblemon_bottlestate_t;

/* Bubble layers */
typedef enum { BACKGROUND, FOREGROUND } bubblemon_layer_t;

/* An (a)rgb color value */
typedef union {
  int value;
  struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
  } components;
} bubblemon_color_t;

/* A bubblemon image */
typedef struct
{
  int width;
  int height;

  bubblemon_colorcode_t *airAndWater;
  bubblemon_color_t *pixels;
} bubblemon_picture_t;

/* A bubble */
typedef struct
{
  int x;
  float y;
  float dy;

  bubblemon_layer_t layer;
} bubblemon_Bubble;

/* A water level */
typedef struct
{
  float y;
  float dy;
} bubblemon_WaterLevel;

/* A weed */
typedef struct
{
  float height;
  float nourishment;

  bubblemon_color_t color;
} bubblemon_Weed;

/* Physics stuff */
typedef struct
{
  bubblemon_WaterLevel *waterLevels;

  int n_bubbles;
  int max_bubbles;
  bubblemon_Bubble *bubbles;
  bubblemon_Weed *weeds;

  float bottle_y;
  float bottle_dy;
  bubblemon_bottlestate_t bottle_state;
} bubblemon_Physics;

/* Data holder for a bubblemon instance. */
typedef struct
{
  char *tooltipstring;

  int physicalTimeElapsed;

  // When bubblemon_getMsecsSinceLastCall was last called
  long last_sec;
  long last_usec;

  // Milliseconds left until the weeds should be updated
  int timeToNextWeedsUpdate;

  // How many bubbles to create per CPU
  float *createNNewBubbles;

  // The CPU loads, sorted
  int *sortedCpuLoads;

  // Weed color striping seed
  int stripey;

  // The picture we're supposed to keep up-to-date
  bubblemon_picture_t bubblePic;

  // Our universe
  bubblemon_Physics physics;

  // System load
  meter_sysload_t sysload;
  
  // Color scheme
  unsigned int noSwapAirColor;
  unsigned int noSwapWaterColor;
  unsigned int maxSwapAirColor;
  unsigned int maxSwapWaterColor;
  unsigned int weedColor0;
  unsigned int weedColor1;
} bubblemon_t;

/* The 'pixels' field of the returned struct contains the pixels to
 * draw on screen. */
const bubblemon_picture_t *bubblemon_getPicture(bubblemon_t *bubblemon);

/* Set the dimensions of the bubble array */
extern void bubblemon_setSize(bubblemon_t *bubblemon, int width, int height);

/* Return how many percent of the memory is used */
extern int bubblemon_getMemoryPercentage(bubblemon_t *bubblemon);

/* Return how many percent of the swap is used */
extern int bubblemon_getSwapPercentage(bubblemon_t *bubblemon);

/* The cpu parameter is the cpu number, 1 - #CPUs.  0 means average load */
extern int bubblemon_getAverageLoadPercentage(bubblemon_t *bubblemon);
extern int bubblemon_getCpuLoadPercentage(bubblemon_t *bubblemon, int cpu);

/* Battery charge left in percent. On a battery-less system, this
   function will return 100%. */
extern int bubblemon_getBatteryChargePercentage(bubblemon_t *bubblemon);

/* Return a suitable tool tip */
extern const char *bubblemon_getTooltip(bubblemon_t *bubblemon);

/* Must be called at the very start of the program */
extern bubblemon_t *bubblemon_init(void);

/* Should be called at shutdown */
extern void bubblemon_done(bubblemon_t *bubblemon);

/* Run self tests, assert()s will fail on failures */
extern void bubblemon_selftest(void);

#endif
