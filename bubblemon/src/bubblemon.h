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

#ifndef BUBBLEMON_H
#define BUBBLEMON_H

#include <sys/types.h>

/* How fast do the bubbles rise? */
#define GRAVITY -0.01

/* How fast do the water levels accelerate? */
#define VOLATILITY 1.0

/* 0.0 means the liquid never moves.
   1.0 means the liquid will continue to oscillate forever. */
#define VISCOSITY 0.98

/* How fast are the water levels allowed to move? */
#define SPEED_LIMIT 1.0

/* The applet's dimensions */
#define RELATIVE_WIDTH 32
#define RELATIVE_HEIGHT 40

/* Swap usage color scale */              /*   rrggbb */
#define NOSWAPAIRCOLOR    (bubblemon_color_t)0x2299ff
#define NOSWAPWATERCOLOR  (bubblemon_color_t)0x0055ff

#define MAXSWAPAIRCOLOR   (bubblemon_color_t)0xff0000
#define MAXSWAPWATERCOLOR (bubblemon_color_t)0xaa0000

/* Water non-transparency, 0-100 */
#define WATERALPHA        70

/* Color code constants */
typedef enum { WATER, ANTIALIAS, AIR } bubblemon_colorcode_t;

/* Bottle behaviour */
typedef enum { GONE, FLOATING, SINKING, FALLING } bubblemon_bottlestate_t;

/* An (a)rgb color value */
typedef union {
  int value;
#ifdef WORDS_BIGENDIAN
  struct {
    unsigned char a;
    unsigned char r;
    unsigned char g;
    unsigned char b;
  } components;
#else
  struct {
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
  } components;
#endif
} bubblemon_color_t;

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
} bubblemon_Bubble;

/* A water level */
typedef struct
{
  float y;
  float dy;
} bubblemon_WaterLevel;

/* Physics stuff */
typedef struct
{
  bubblemon_WaterLevel *waterLevels;

  int n_bubbles;
  int max_bubbles;
  bubblemon_Bubble *bubbles;

  float bottle_y;
  bubblemon_bottlestate_t bottle_state;
} bubblemon_Physics;

/* The 'pixels' field of the returned struct contains the pixels to
 * draw on screen. */
const bubblemon_picture_t *bubblemon_getPicture(void);

/* Set the dimensions of the bubble array */
extern void bubblemon_setSize(int width, int height);

/* Return how many percent of the memory is used */
extern int bubblemon_getMemoryPercentage(void);

/* Return how many percent of the swap is used */
extern int bubblemon_getSwapPercentage(void);

/* The cpu parameter is the cpu number, 1 - #CPUs.  0 means average load */
extern int bubblemon_getAverageLoadPercentage(void);
extern int bubblemon_getCpuLoadPercentage(int cpu);

/* Return a suitable tool tip */
extern const char *bubblemon_getTooltip(void);

#endif
