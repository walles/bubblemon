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
 * This file contains the GNOME ui for bubblemon.  It has been adapted
 * from the life applet that comes with GNOME 1.4.  That applet is
 * copyrighted to "The man in the box" (whoever that is).
 */

#include "config.h"

#include "ui.h"
#include "bubblemon.h"

#include <gtk/gtk.h>
#include <applet-widget.h>

#define FRAMERATE 25

static int width;
static int height;
static GtkWidget *drawingArea = NULL;
static GtkWidget *applet;

static guchar *rgb_buffer;

static void ui_update(void)
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

  bubblePic = bubblemon_getPicture();
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
ui_expose(void)
{
  ui_update();
  return FALSE;
}

static int
ui_timeoutHandler(gpointer ignored)
{
  ui_update();
  return TRUE;
}

static GtkWidget *ui_create(void)
{
  GtkWidget *frame;
  
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  
  gtk_widget_push_visual (gdk_rgb_get_visual ());
  gtk_widget_push_colormap (gdk_rgb_get_cmap ());
  drawingArea = gtk_drawing_area_new();
  g_assert(drawingArea != NULL);
  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();
  
  gtk_widget_set_events (drawingArea,
			 GDK_EXPOSURE_MASK |
			 GDK_ENTER_NOTIFY_MASK);

  gtk_drawing_area_size(GTK_DRAWING_AREA(drawingArea), width, height);
  
  gtk_container_add (GTK_CONTAINER (frame), drawingArea);
  gtk_widget_show (drawingArea);
  gtk_widget_show (frame);

  gtk_signal_connect_after(GTK_OBJECT(drawingArea), "realize",
			   GTK_SIGNAL_FUNC(ui_update), NULL);
  gtk_signal_connect(GTK_OBJECT(drawingArea), "expose_event",
		     GTK_SIGNAL_FUNC(ui_expose), NULL);

  return frame;
}

static void about (AppletWidget *applet, gpointer data)
{
  static const char *authors[] = { "Johan Walles <d92-jwa@nada.kth.se>", NULL };
  static GtkWidget *about_box = NULL;

  if (about_box != NULL)
  {
    gdk_window_show(about_box->window);
    gdk_window_raise(about_box->window);
    return;
  }
  about_box = gnome_about_new (_("Bubbling Load Monitor"), VERSION,
			       _("Copyright (C) 1999-2002 Johan Walles"),
			       authors,
			       _("This applet displays your CPU load as a bubbling liquid.\n"
				 "This applet comes with ABSOLUTELY NO WARRANTY, "
				 "see the LICENSE file for details.\n"
				 "This is free software, and you are welcome to redistribute it "
				 "under certain conditions (GPL), "
				 "see the LICENSE file for details."),
			       NULL);
  
  gtk_signal_connect( GTK_OBJECT(about_box), "destroy",
		      GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about_box );
  gtk_widget_show(about_box);
  return;
}

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

static void applet_change_pixel_size(GtkWidget *ignored1,
				     int newSize,
				     gpointer ignored2)
{
  ui_setSize(newSize);
  ui_update();
}

// FIXME: Update this to reflect bubblemon info
/*
static void help_cb (AppletWidget *applet, gpointer data)
{
  GnomeHelpMenuEntry help_entry = { "life_applet", "index.html"};
  gnome_help_display(NULL, &help_entry);
}
*/

static void update_tooltip()
{
  /* This is part of a workaround for the gtk+ tool tip problem. */
  applet_widget_set_widget_tooltip(APPLET_WIDGET(applet),
				   GTK_WIDGET(drawingArea),
				   bubblemon_getTooltip());
}

static void applet_leave_cb (GtkWidget *ignored1,
		      GdkEventAny *ignored2,
		      gpointer data)
{
  /* This is part of a workaround for a gtk+ tool tip problem. */
  update_tooltip();
}

int ui_main(int argc, char *argv[])
{
  GtkWidget *bubblemon;

#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif
  
  applet_widget_init (PACKAGE, VERSION, argc, argv, NULL, 0, NULL);
  // gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-life.png");

  applet = applet_widget_new (PACKAGE);
  if (!applet)
  {
    g_error (_("Can't create bubblemon applet!"));
  }
  
#ifdef HAVE_CHANGE_PIXEL_SIZE
  ui_setSize(applet_widget_get_panel_pixel_size(APPLET_WIDGET(applet)));

  gtk_signal_connect(GTK_OBJECT(applet),"change_pixel_size",
		     GTK_SIGNAL_FUNC(applet_change_pixel_size),
		     NULL);
#else
  ui_setSize(48);
#endif

  bubblemon = ui_create();
  
  /* Add a signal to the applet for when the mouse exits to update the tooltip */
  gtk_signal_connect (GTK_OBJECT (applet), "leave_notify_event",
		      GTK_SIGNAL_FUNC(applet_leave_cb),
		      NULL);
  
  applet_widget_add(APPLET_WIDGET(applet), bubblemon);
  gtk_widget_show(bubblemon);

  gtk_widget_show(applet);

  /*
  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					 "help",
					 GNOME_STOCK_PIXMAP_HELP,
					 _("Help"), help_cb, NULL);
  */
  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _("About..."),
					 about,
					 NULL);

  gtk_timeout_add(1000 / FRAMERATE, ui_timeoutHandler, NULL);

  applet_widget_gtk_main ();

  return 0;
}
