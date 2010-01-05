/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2010 Johan Walles - johan.walles@gmail.com
 *  This file (C) 2002-2008 Juan Salaverria - rael@vectorstar.net
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
 * This file contains the GNOME 2 ui for bubblemon. It has been
 * adapted from many GNOME 2 core applets already ported, and based in
 * the original gnome1-ui.c code, of course.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

#include <config.h>
#include <math.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include "gnome2-ui.h"
#include "meter.h"
#include "mail.h"
#include "bubblemon.h"

// Bottle graphics
#include "msgInBottle.c"

static void
display_about_dialog (BonoboUIComponent *uic,
		      gpointer data,
		      const gchar *verbname)
{
  // BubblemonApplet *bubble = (BubblemonApplet*)data;

  static const gchar *authors[] = { "Johan Walles <johan.walles@gmail.com>",
				    "Juan Salaverria <rael@vectorstar.net>",
				    NULL };

  gtk_show_about_dialog (NULL,
			 "version",            VERSION,
			 "copyright",          "Copyright (C) 1999-2010 Johan Walles",
			 "comments",           _("Displays system load as a bubbling liquid."),
			 "authors",            authors,
			 // "translator-credits", _("translator-credits"),
			 // "logo_icon_name",     NULL,
			 NULL);
}

static void
ui_update (BubblemonApplet *applet)
{
  int w, h, i;
  const bubblemon_picture_t *bubblePic;
  bubblemon_color_t *pixel;
  guchar *p;

  GdkGC *gc;

  GtkWidget *drawingArea = applet->drawingArea;

  if((drawingArea == NULL) ||
     !GTK_WIDGET_REALIZED(drawingArea) ||
     !GTK_WIDGET_DRAWABLE(drawingArea) ||
     applet->width <= 0)
  {
    return;
  }

  bubblePic = bubblemon_getPicture(applet->bubblemon);
  if ((bubblePic == NULL) ||
      (bubblePic->width == 0) ||
      (bubblePic->pixels == 0))
  {
    return;
  }
  w = bubblePic->width;
  h = bubblePic->height;

  gc = gdk_gc_new(drawingArea->window);

  p = applet->rgb_buffer;
  pixel = bubblePic->pixels;
  for(i = 0; i < w * h; i++) {
    *(p++) = pixel->components.r;
    *(p++) = pixel->components.g;
    *(p++) = pixel->components.b;
    pixel++;
  }

  gdk_draw_rgb_image(drawingArea->window, gc,
                     0, 0,
		     applet->width, applet->height,
                     GDK_RGB_DITHER_NORMAL,
                     applet->rgb_buffer, w * 3);

  gdk_gc_destroy(gc);

}

static int
ui_expose(GtkWidget *exposed, GdkEventExpose *event, gpointer data)
{
  BubblemonApplet *applet = (BubblemonApplet*)data;

  ui_update(applet);
  return FALSE;
}

static int
ui_realize(GtkWidget *realized, gpointer data)
{
  BubblemonApplet *applet = (BubblemonApplet*)data;

  ui_update(applet);
  return FALSE;
}

static int
ui_timeoutHandler(gpointer data)
{
  BubblemonApplet *applet = (BubblemonApplet*)data;

  ui_update(applet);
  return TRUE;
}

static int
update_tooltip (gpointer bubbles)
{
  BubblemonApplet *bubble = bubbles;

  gtk_widget_set_tooltip_text(bubble->applet,
			      bubblemon_getTooltip(bubble->bubblemon));

  // FIXME: We want to call
  // gtk_widget_trigger_tooltip_query(bubble->applet) here, but we can't
  // due to http://bugs.debian.org/510873.
  //
  // There may be other problems as well, try running the applet in
  // valgrind after 510873 is fixed and see if we can show the tooltip
  // without any complaints from valgrind then.

  return TRUE;
}

static void
applet_destroy (GtkWidget *panelApplet, BubblemonApplet *applet)
{
  g_source_remove(applet->refresh_timeout_id);
  applet->refresh_timeout_id = 0;
  g_source_remove(applet->tooltip_timeout_id);
  applet->tooltip_timeout_id = 0;

  if (applet->rgb_buffer != NULL) {
    g_free(applet->rgb_buffer);
  }
  applet->rgb_buffer = NULL;

  bubblemon_done(applet->bubblemon);

  g_free(applet);
}

static gboolean
applet_reconfigure (GtkDrawingArea *drawingArea, GdkEventConfigure *event, BubblemonApplet *bubble)
{
  int width = event->width;
  int height = event->height;

  PanelAppletOrient orientation =
    panel_applet_get_orient(PANEL_APPLET(bubble->applet));

  if (orientation == PANEL_APPLET_ORIENT_LEFT
    || orientation == PANEL_APPLET_ORIENT_RIGHT)
  {
    // We're on a vertical panel, height is decided based on the width
    if (width <= RELATIVE_WIDTH) {
      height = RELATIVE_HEIGHT;
    } else {
      height = (width * RELATIVE_HEIGHT) / RELATIVE_WIDTH;
    }
  } else {
    // We're on a horizontal panel, width is decided based on the height
    if (height <= RELATIVE_HEIGHT) {
      width = RELATIVE_WIDTH;
    } else {
      width = (height * RELATIVE_WIDTH) / RELATIVE_HEIGHT;
    }
  }

  if (bubble->width == width
      && bubble->height == height)
  {
    // Already at the correct size, done!
    return TRUE;
  }

  gtk_widget_set_size_request(GTK_WIDGET(drawingArea), width, height);

  bubble->width = width;
  bubble->height = height;

  if (bubble->applet == NULL) {
    // Not yet all loaded up
    return TRUE;
  }

  bubble->rgb_buffer = g_realloc(bubble->rgb_buffer, width * height * 3);
  bubblemon_setSize(bubble->bubblemon, width, height);

  ui_update(bubble);

  return TRUE;
}

static const BonoboUIVerb bubblemon_menu_verbs [] = {
  BONOBO_UI_VERB ("About", display_about_dialog),
  BONOBO_UI_VERB_END
};

static gboolean
bubblemon_applet_fill(BubblemonApplet *bubblemon_applet, PanelApplet *applet)
{
  GtkWidget *drawingArea;

  panel_applet_set_flags(applet, PANEL_APPLET_EXPAND_MINOR);

  bubblemon_applet->applet = GTK_WIDGET (applet);
  bubblemon_applet->width   = 0;
  bubblemon_applet->height= 0;

  g_signal_connect (G_OBJECT (bubblemon_applet->applet),
		    "destroy",
		    G_CALLBACK (applet_destroy),
		    bubblemon_applet);

  drawingArea = gtk_drawing_area_new();
  g_assert(drawingArea != NULL);
  bubblemon_applet->drawingArea = drawingArea;
  gtk_widget_set_size_request(GTK_WIDGET(drawingArea), RELATIVE_WIDTH, RELATIVE_HEIGHT);

  g_signal_connect (G_OBJECT (drawingArea),
		    "configure_event",
		    G_CALLBACK (applet_reconfigure),
		    bubblemon_applet);

  gtk_widget_set_events(drawingArea,
			GDK_EXPOSURE_MASK
			| GDK_ENTER_NOTIFY_MASK
			| GDK_STRUCTURE_MASK);

  gtk_container_add(GTK_CONTAINER (bubblemon_applet->applet), drawingArea);

  gtk_signal_connect_after(GTK_OBJECT(drawingArea), "realize",
			   GTK_SIGNAL_FUNC(ui_realize), bubblemon_applet);

  gtk_signal_connect(GTK_OBJECT(drawingArea), "expose_event",
		     GTK_SIGNAL_FUNC(ui_expose), bubblemon_applet);

  gtk_widget_show_all (GTK_WIDGET (bubblemon_applet->applet));

  panel_applet_setup_menu_from_file (PANEL_APPLET (bubblemon_applet->applet),
				     NULL,
				     "GNOME_BubblemonApplet.xml",
				     NULL,
				     bubblemon_menu_verbs,
				     bubblemon_applet);

  bubblemon_applet->refresh_timeout_id =
    g_timeout_add(1000 / FRAMERATE, ui_timeoutHandler, bubblemon_applet);
  bubblemon_applet->tooltip_timeout_id =
    g_timeout_add(2000, update_tooltip, bubblemon_applet);

  return TRUE;
}

static gboolean
bubble_applet_factory (PanelApplet *panel_applet,
		       const gchar *iid,
		       gpointer     data)
{
  BubblemonApplet *bubblemon_applet;

  gboolean retval = FALSE;

  bubblemon_applet = g_new0(BubblemonApplet, 1);
  bubblemon_applet->bubblemon = bubblemon_init();

  if (strcmp(iid, "OAFIID:GNOME_BubblemonApplet") == 0) {
    retval = bubblemon_applet_fill(bubblemon_applet, panel_applet);
  }

  return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_BubblemonApplet_Factory",
			     PANEL_TYPE_APPLET,
			     PACKAGE_NAME,
			     VERSION,
			     bubble_applet_factory,
			     NULL)
