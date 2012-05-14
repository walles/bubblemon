/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2010 Johan Walles - johan.walles@gmail.com
 *  This file (C) 2012 Adam Sjøgren - asjo@koldfront.dk
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
 * This file contains the XFCE4 user interface for bubblemon. It has
 * been adapted from the CPU graph plugin for XFCE, and the
 * gnome2-ui.c code.
 *
 */

#include "xfce4-ui.h"

#include <libxfcegui4/libxfcegui4.h>
#include <libxfce4panel/xfce-hvbox.h>
#ifndef _
# include <libintl.h>
# define _(String) gettext (String)
#endif

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL(bubblemon_construct);

static void
display_about_dialog (XfcePanelPlugin *plugin, Bubblemon *base)
{
  static const gchar *authors[] = { "Johan Walles <johan.walles@gmail.com>",
				    "Juan Salaverria <rael@vectorstar.net>",
				    NULL };

  gtk_show_about_dialog (NULL,
			 "version",            VERSION,
			 "copyright",          "Copyright (C) 1999-2012 Johan Walles",
			 "comments",           _("Displays system load as a bubbling liquid."),
			 "authors",            authors,
			 NULL);
}

static gboolean
ui_update(Bubblemon *base)
{
  int w, h, i;
  const bubblemon_picture_t *bubblePic;
  bubblemon_color_t *pixel;
  guchar *p;

  GdkGC *gc;

  GtkWidget *draw_area = base->draw_area;

  if((draw_area == NULL) ||
     !GTK_WIDGET_REALIZED(draw_area) ||
     !GTK_WIDGET_DRAWABLE(draw_area) ||
     base->width <= 0)
    {
      return TRUE;
    }

  bubblePic = bubblemon_getPicture(base->bubblemon);
  if ((bubblePic == NULL) ||
      (bubblePic->width == 0) ||
      (bubblePic->pixels == 0))
    {
      return TRUE;
    }
  w = bubblePic->width;
  h = bubblePic->height;

  gc = gdk_gc_new(draw_area->window);

  p = base->rgb_buffer;
  pixel = bubblePic->pixels;
  for(i = 0; i < w * h; i++) {
    *(p++) = pixel->components.r;
    *(p++) = pixel->components.g;
    *(p++) = pixel->components.b;
    pixel++;
  }

  gdk_draw_rgb_image(draw_area->window, gc,
                     0, 0,
                     base->width, base->height,
                     GDK_RGB_DITHER_NORMAL,
                     base->rgb_buffer, w * 3);

  gdk_gc_destroy(gc);

  return TRUE;
}

static void
plugin_destroy (XfcePanelPlugin *plugin, Bubblemon *base)
{
  g_source_remove(base->refresh_timeout_id);
  base->refresh_timeout_id = 0;
  g_source_remove(base->tooltip_timeout_id);
  base->tooltip_timeout_id = 0;

  if (base->rgb_buffer != NULL) {
    g_free(base->rgb_buffer);
  }
  base->rgb_buffer = NULL;

  bubblemon_done(base->bubblemon);

  g_free(base);
}

static gboolean
plugin_reconfigure (XfcePanelPlugin *plugin, guint size, Bubblemon *base)
{
  int width = base->width;
  int height = base->height;

  GtkOrientation orientation =
	  xfce_panel_plugin_get_orientation(base->plugin);

  if (orientation != GTK_ORIENTATION_HORIZONTAL)
  {
    width=size;
    // We're on a vertical panel, height is decided based on the width
    if (width <= RELATIVE_WIDTH) {
      height = RELATIVE_HEIGHT;
    } else {
      height = (width * RELATIVE_HEIGHT) / RELATIVE_WIDTH;
    }
  } else {
    height=size;
    // We're on a horizontal panel, width is decided based on the height
    if (height <= RELATIVE_HEIGHT) {
      width = RELATIVE_WIDTH;
    } else {
      width = (height * RELATIVE_WIDTH) / RELATIVE_HEIGHT;
    }
  }

  if (base->width == width
      && base->height == height)
  {
    // Already at the correct size, done!
    return TRUE;
  }

  gtk_widget_set_size_request(GTK_WIDGET(base->draw_area), width, height);

  base->width = width;
  base->height = height;

  if (base->plugin == NULL) {
    // Not yet all loaded up
    return TRUE;
  }

  base->rgb_buffer = g_realloc(base->rgb_buffer, width * height * 3);
  bubblemon_setSize(base->bubblemon, width, height);

  ui_update(base);

  return TRUE;
}

static void
update_draw_area (GtkWidget *da, GdkEventExpose *event, gpointer data)
{
  ui_update((Bubblemon *)data);
}

static int
ui_timeoutHandler (gpointer data)
{
  Bubblemon *base = (Bubblemon*)data;

  ui_update(base);
  return TRUE;
}

static gboolean
update_tooltip (gpointer data)
{
  Bubblemon *base = data;

  gtk_widget_set_tooltip_text(base->frame_widget,
                              bubblemon_getTooltip(base->bubblemon));

  return TRUE;
}

static gboolean
bubblemon_plugin_fill(Bubblemon *base, XfcePanelPlugin *plugin)
{
  GtkWidget *draw_area;

  base->width  = 0;
  base->height = 0;

  draw_area = gtk_drawing_area_new();
  g_assert(draw_area != NULL);
  base->draw_area = draw_area;
  gtk_widget_set_size_request(GTK_WIDGET(draw_area), RELATIVE_WIDTH, RELATIVE_HEIGHT);

  gtk_widget_set_events(draw_area,
			GDK_EXPOSURE_MASK
			| GDK_ENTER_NOTIFY_MASK
			| GDK_STRUCTURE_MASK);

  gtk_signal_connect_after(GTK_OBJECT(draw_area), "expose_event",
		     GTK_SIGNAL_FUNC(update_draw_area), base);

  base->refresh_timeout_id =
    g_timeout_add(1000 / FRAMERATE, ui_timeoutHandler, base);

  base->tooltip_timeout_id =
    g_timeout_add(2000, update_tooltip, base);

  return TRUE;
}

static void
update_orientation (XfcePanelPlugin *plugin, GtkOrientation orientation, Bubblemon *base)
{
  xfce_hvbox_set_orientation(XFCE_HVBOX(base->box), orientation);
}

static Bubblemon *
create_gui (XfcePanelPlugin *plugin)
{
  GtkWidget *frame, *ebox;
  GtkOrientation orientation;
  Bubblemon *base = g_new0(Bubblemon, 1);

  orientation = xfce_panel_plugin_get_orientation(plugin);
  base->plugin=plugin;

  base->bubblemon=bubblemon_init();
  bubblemon_plugin_fill(base, plugin);

  ebox = gtk_event_box_new();
  gtk_container_add( GTK_CONTAINER( plugin ), ebox );

  base->box = xfce_hvbox_new(orientation, FALSE, 0);
  gtk_container_add(GTK_CONTAINER(ebox), base->box);

  base->frame_widget = frame = gtk_frame_new( NULL );
  gtk_box_pack_end( GTK_BOX(base->box), frame, TRUE, TRUE, 0);

  gtk_container_add( GTK_CONTAINER( frame ), GTK_WIDGET( base->draw_area ) );

  update_orientation(plugin, orientation, base);
  gtk_widget_show_all(ebox);

  return base;
}

static void
bubblemon_construct (XfcePanelPlugin *plugin)
{
  Bubblemon *base;

  xfce_textdomain( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  base = create_gui(plugin);
  xfce_panel_plugin_menu_show_configure(plugin);

  g_signal_connect(plugin, "free-data", G_CALLBACK(plugin_destroy), base);
  g_signal_connect(plugin, "configure-plugin", G_CALLBACK(display_about_dialog), base);
  g_signal_connect(plugin, "size-changed", G_CALLBACK(plugin_reconfigure), base);
  g_signal_connect(plugin, "orientation-changed", G_CALLBACK(update_orientation), base);
}
