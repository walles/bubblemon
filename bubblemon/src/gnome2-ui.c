/*
 *  Bubbling Load Monitoring Applet
 *  Copyright (C) 1999-2002 Johan Walles - d92-jwa@nada.kth.se
 *  This file (C) 2002 Juan Salaverria - rael@vectorstar.net
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
#include <gnome.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>

#include <libgnome/gnome-help.h>

#include "gnome2-ui.h"
#include "meter.h"
#include "mail.h"
#include "bubblemon.h"
#include "bubblemon-about-dialog.h"
#include "bubblemon-prefs-dialog.h"

// Bottle graphics
#include "msgInBottle.c"

static int width;
static int height;
static GtkWidget *drawingArea = NULL;
static GtkWidget *applet;

static guchar *rgb_buffer;


static void ui_setSize(int newPanelSize)
{
  // Leave room for the border
  int newSize = newPanelSize - 4;

  height = newSize;
  width  = (newSize * RELATIVE_WIDTH) / RELATIVE_HEIGHT;

  /*
    FIXME: For some unknown reason, at 16bpp, the width cannot be odd,
    or the drawing doesn't work.  I have not been able to determine
    why.  Until someone convinces me otherwise, I'll assume this is a
    bug in gdk / gtk+.  Anyway, the workaround on the next line kills
    the lowermost bit of the new width so that this bug never (?) gets
    triggered.  This is not a solution, and I hate it, but it's the
    best I'll do for the moment.
  */
  width &= ~1;

  if (drawingArea != NULL)
  {
    gtk_drawing_area_size(GTK_DRAWING_AREA(drawingArea), width, height);
    gtk_widget_set_usize(GTK_WIDGET(drawingArea), width, height);
  }

  rgb_buffer = realloc(rgb_buffer, width * height * 3);

  bubblemon_setSize(width, height);
}


static void
ui_update (BubblemonApplet *bubble)
{
  int w, h, i;
  const bubblemon_picture_t *bubblePic;
  bubblemon_color_t *pixel;
  guchar *p;

  GdkGC *gc;

  if((drawingArea == NULL) ||
     !GTK_WIDGET_REALIZED(drawingArea) ||
     !GTK_WIDGET_DRAWABLE(drawingArea) ||
     width <= 0)
  {
    return;
  }

  bubblePic = bubblemon_getPicture(bubble);
  if ((bubblePic == NULL) ||
      (bubblePic->width == 0) ||
      (bubblePic->pixels == 0))
  {
    return;
  }
  w = bubblePic->width;
  h = bubblePic->height;

  gc = gdk_gc_new(drawingArea->window);

  p = rgb_buffer;
  pixel = bubblePic->pixels;
  for(i = 0; i < w * h; i++) {
    *(p++) = pixel->components.r;
    *(p++) = pixel->components.g;
    *(p++) = pixel->components.b;
    pixel++;
  }

  gdk_draw_rgb_image(drawingArea->window, gc,
                     0, 0, width, height,
                     GDK_RGB_DITHER_NORMAL,
                     rgb_buffer, w * 3);

  gdk_gc_destroy(gc);

}

static int
ui_expose (BubblemonApplet *bubble)
{
  ui_update(bubble);
  return FALSE;
}

static int
ui_timeoutHandler(gpointer bubbles)
{
BubblemonApplet *bubble = bubbles;

  ui_update(bubble);
  return TRUE;
}

static void
set_tooltip (gpointer bubbles) 
{
  GtkTooltips *tooltips;
  BubblemonApplet *bubble = bubbles;
	
  tooltips = gtk_tooltips_new ();
  g_object_ref (tooltips);
  gtk_object_sink (GTK_OBJECT (tooltips));
  g_object_set_data (G_OBJECT (bubble->applet), "tooltips", tooltips);
  gtk_tooltips_set_tip (tooltips, bubble->applet, bubblemon_getTooltip(), NULL);
  bubble->tooltips = tooltips;
}

static int 
update_tooltip (gpointer bubbles) 
{
  BubblemonApplet *bubble = bubbles;
	
  gtk_tooltips_set_tip (bubble->tooltips, bubble->applet, bubblemon_getTooltip(), NULL);
  return TRUE;
}


void
destroy_tooltip (GtkWidget *object)
{
  GtkTooltips *tooltips;

  tooltips = g_object_get_data (G_OBJECT (object), "tooltips");
  if (tooltips) {
    g_object_unref (tooltips);
    g_object_set_data (G_OBJECT (object), "tooltips", NULL);
  }
}


static void
applet_destroy (GtkWidget *applet, BubblemonApplet *bubble)
{
  destroy_tooltip (GTK_WIDGET (bubble->applet));

  if (bubble->aboutbox != NULL)
    gtk_widget_destroy(bubble->aboutbox);
  bubble->aboutbox = NULL;

  g_free(bubble);

  bubblemon_done();
}


static void
applet_change_size (PanelApplet *applet, gint  size, gpointer data)
{
  BubblemonApplet *bubble = (BubblemonApplet *)data;

  if (bubble->size == size)
    return;

  bubble->size = size;

  /* not yet all loaded up */
  if (bubble->frame == NULL)
    return;

  ui_setSize(size);
  ui_update(bubble);
}

static void pref_cb (BonoboUIComponent *uic,
                      	BubblemonApplet *bubble,
			const gchar       *verbname)
{
  bubblemon_prefs_run (bubble);
}

static void about_cb (BonoboUIComponent *uic,
                      	BubblemonApplet *bubble,
			const gchar       *verbname)
{
  bubblemon_about_run (bubble);
}

static void help_cb (BonoboUIComponent *uic,
                      	BubblemonApplet *bubble,
			const gchar       *verbname)
{

  GError *error = NULL;

  egg_screen_help_display (gtk_widget_get_screen (GTK_WIDGET (bubble->applet)),
				"bubblemon", NULL, &error);

  if (error) { /* FIXME: the user needs to see this error */
	g_print ("%s \n", error->message);
	g_error_free (error);
	error = NULL;
  }

}


static void
gconf_notify_func_netload (GConfClient *client,
			guint        cnxn_id,
			GConfEntry  *entry,
			gpointer     user_data)
{
BubblemonApplet *bubble_applet = user_data;
GConfValue  *value;

  value = gconf_entry_get_value (entry);
  bubble_applet->pref.gnetload = gconf_value_get_bool (value);

  update_bubblemon_pref_windows(bubble_applet);
}

static void
gconf_notify_func_mailcheck (GConfClient *client,
			guint        cnxn_id,
			GConfEntry  *entry,
			gpointer     user_data)
{
BubblemonApplet *bubble_applet = user_data;
GConfValue  *value;

  value = gconf_entry_get_value (entry);
  bubble_applet->pref.gmailcheck = gconf_value_get_bool (value);

  update_bubblemon_pref_windows(bubble_applet);
}

static const BonoboUIVerb bubblemon_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("Properties", pref_cb),
	BONOBO_UI_UNSAFE_VERB ("Help", help_cb),
	BONOBO_UI_UNSAFE_VERB ("About", about_cb),
  	BONOBO_UI_VERB_END
};

static gboolean
bubblemon_applet_fill (PanelApplet *applet)
{
	
  BubblemonApplet *bubblemon_applet;
  GConfClient *client;
	
  bubblemon_applet = g_new0 (BubblemonApplet, 1);

  bubblemon_applet->applet = GTK_WIDGET (applet);
  bubblemon_applet->size   = panel_applet_get_size (applet);

  panel_applet_add_preferences (PANEL_APPLET(bubblemon_applet->applet), "/schemas/apps/bubblemon", NULL);
  client = gconf_client_get_default ();
  bubblemon_applet->pref.gnetload = gconf_client_get_bool (client, GC_KEY_NETLOAD, NULL);
  bubblemon_applet->pref.gmailcheck = gconf_client_get_bool (client, GC_KEY_MAILCHECK, NULL);

  gconf_client_add_dir (client,
			GC_PATH,
			GCONF_CLIENT_PRELOAD_NONE,
			NULL);

  gconf_client_notify_add (client,
			GC_KEY_NETLOAD,
			gconf_notify_func_netload,
			bubblemon_applet,
			NULL,
			NULL);

  gconf_client_notify_add (client,
			GC_KEY_MAILCHECK,
			gconf_notify_func_mailcheck,
			bubblemon_applet,
			NULL,
			NULL);

  g_signal_connect (G_OBJECT (bubblemon_applet->applet),
		    "destroy",
		    G_CALLBACK (applet_destroy),
		    bubblemon_applet);

  g_signal_connect (G_OBJECT (bubblemon_applet->applet),
		    "change_size",
		    G_CALLBACK (applet_change_size),
		    bubblemon_applet);

  ui_setSize(panel_applet_get_size(PANEL_APPLET(bubblemon_applet->applet)));

  bubblemon_applet->frame = gtk_frame_new (NULL);	
  gtk_frame_set_shadow_type (GTK_FRAME (bubblemon_applet->frame), GTK_SHADOW_IN);

  gtk_widget_push_visual(gdk_rgb_get_visual());
  gtk_widget_push_colormap(gdk_rgb_get_cmap());
  
  drawingArea = gtk_drawing_area_new();
  g_assert(drawingArea != NULL);

  gtk_widget_pop_colormap();
  gtk_widget_pop_visual();
  
  gtk_widget_set_events(drawingArea,
			GDK_EXPOSURE_MASK |
			GDK_ENTER_NOTIFY_MASK);
        
  gtk_drawing_area_size(GTK_DRAWING_AREA(drawingArea), width, height);
 
  gtk_container_add(GTK_CONTAINER (bubblemon_applet->frame), drawingArea);
  gtk_widget_show(drawingArea);
  gtk_widget_show(bubblemon_applet->frame);
        
  gtk_signal_connect_after(GTK_OBJECT(drawingArea), "realize",
			   GTK_SIGNAL_FUNC(ui_update), bubblemon_applet);
        
  gtk_signal_connect(GTK_OBJECT(drawingArea), "expose_event",
		     GTK_SIGNAL_FUNC(ui_expose), bubblemon_applet);
        
  set_tooltip (bubblemon_applet);

  gtk_container_add (GTK_CONTAINER (bubblemon_applet->applet), bubblemon_applet->frame);

  gtk_widget_show_all (GTK_WIDGET (bubblemon_applet->frame));

  gtk_widget_show (GTK_WIDGET (bubblemon_applet->applet));

  panel_applet_setup_menu_from_file (PANEL_APPLET (bubblemon_applet->applet),
				     NULL,
				     "GNOME_BubblemonApplet.xml",
				     NULL,
				     bubblemon_menu_verbs,
				     bubblemon_applet);
	
  gtk_timeout_add(1000 / FRAMERATE, ui_timeoutHandler, bubblemon_applet);
  gtk_timeout_add(2000, update_tooltip, bubblemon_applet);
	
  return TRUE;
}

static gboolean
bubble_applet_factory (PanelApplet *applet,
		       const gchar *iid,
		       gpointer     data)
{
  gboolean retval = FALSE;
  
  // Initialize the load metering
  bubblemon_init();
  
  if (strcmp(iid, "OAFIID:GNOME_BubblemonApplet") == 0)
    retval = bubblemon_applet_fill (applet); 
  
  return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_BubblemonApplet_Factory",
			     PANEL_TYPE_APPLET,
			     "bubblemon",
			     "0",
			     bubble_applet_factory,
			     NULL)
