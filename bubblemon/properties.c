/*
 *  Bubbling Load Monitoring Applet
 *  - A GNOME panel applet that displays the CPU + memory load as a
 *    bubbling liquid.
 *  Copyright (C) 1999 Johan Walles
 *  - d92-jwa@nada.kth.se
 *  Copyright (C) 1999 Merlin Hughes
 *  - merlin@merlin.org
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

/* #include <config.h> */
#include <stdio.h>
#include <gnome.h>
#include "bubblemon.h"
#include "properties.h"

static int prop_cancel (GtkWidget * w, gpointer data);
static void prop_apply (GtkWidget *w, int page, gpointer data);

/* Create the properties window */
void
bubblemon_properties_window (AppletWidget * applet, gpointer data)
{
  static GnomeHelpMenuEntry help_entry = { NULL, "properties" };
  BubbleMonData * mc = data; 
  GtkWidget * t, * l, * r2;
  GtkWidget * breadth_spin, * depth_spin, * update_spin, * samples_spin;
  int r, g, b;

  help_entry.name = gnome_app_id;

  if (mc->prop_win)
    {
      gtk_widget_show (GTK_WIDGET (mc->prop_win));
      gdk_window_raise (GTK_WIDGET (mc->prop_win)->window);

      return;
    }
	
  mc->prop_win = GNOME_PROPERTY_BOX (gnome_property_box_new ());

  gnome_dialog_close_hides
    (GNOME_DIALOG (& (mc->prop_win->dialog)), TRUE);

  gtk_window_set_title (
	GTK_WINDOW (&GNOME_PROPERTY_BOX (mc->prop_win)->dialog.window),
	_("Bubbling Load Monitor Settings"));
  
  /*
   *
   * General Properties
   *
   */
  t = gtk_table_new (0, 0, FALSE);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (mc->prop_win), t,
				  gtk_label_new (_("General")));

  /* Applet values */
  l = gtk_label_new (_("Width:")); 
  gtk_table_attach_defaults ( GTK_TABLE (t), l, 0, 1, 0, 1 ); 
  mc->breadth_adj = gtk_adjustment_new ( mc->breadth, 1, 666, 1, 8, 8 );
  breadth_spin = gtk_spin_button_new ( GTK_ADJUSTMENT (mc->breadth_adj), 1, 0 );
  gtk_table_attach_defaults ( GTK_TABLE (t), breadth_spin, 1, 2, 0, 1 );
  gtk_spin_button_set_update_policy ( GTK_SPIN_BUTTON (breadth_spin),
				     GTK_UPDATE_ALWAYS );
  gtk_signal_connect (GTK_OBJECT (mc->breadth_adj), "value_changed",
		     GTK_SIGNAL_FUNC (adj_value_changed_cb), mc);

  l = gtk_label_new (_("Height:")); 
  gtk_table_attach_defaults ( GTK_TABLE (t), l, 0, 1, 1, 2 ); 
  mc->depth_adj = gtk_adjustment_new ( mc->depth, 1, 666, 1, 8, 8 );
  depth_spin = gtk_spin_button_new ( GTK_ADJUSTMENT (mc->depth_adj), 1, 0 );
  gtk_table_attach_defaults ( GTK_TABLE (t), depth_spin, 1, 2, 1, 2 );
  gtk_spin_button_set_update_policy ( GTK_SPIN_BUTTON (depth_spin),
				     GTK_UPDATE_ALWAYS );
  gtk_signal_connect (GTK_OBJECT (mc->depth_adj), "value_changed",
		     GTK_SIGNAL_FUNC (adj_value_changed_cb), mc);

  l = gtk_label_new (_("Update period (ms):")); 
  gtk_table_attach_defaults ( GTK_TABLE (t), l, 0, 1, 2, 3 ); 
  mc->update_adj = gtk_adjustment_new ( mc->update, 1, 666, 1, 8, 8 );
  update_spin = gtk_spin_button_new ( GTK_ADJUSTMENT (mc->update_adj), 1, 0 );
  gtk_table_attach_defaults ( GTK_TABLE (t), update_spin, 1, 2, 2, 3 );
  gtk_spin_button_set_update_policy ( GTK_SPIN_BUTTON (update_spin),
				     GTK_UPDATE_ALWAYS );
  gtk_signal_connect (GTK_OBJECT (mc->update_adj), "value_changed",
		     GTK_SIGNAL_FUNC (adj_value_changed_cb), mc);

  l = gtk_label_new (_("Load samples:")); 
  gtk_table_attach_defaults ( GTK_TABLE (t), l, 0, 1, 3, 4 ); 
  mc->samples_adj = gtk_adjustment_new ( mc->samples, 1, 666, 1, 8, 8 );
  samples_spin = gtk_spin_button_new ( GTK_ADJUSTMENT (mc->samples_adj), 1, 0 );
  gtk_table_attach_defaults ( GTK_TABLE (t), samples_spin, 1, 2, 3, 4 );
  gtk_spin_button_set_update_policy ( GTK_SPIN_BUTTON (samples_spin),
				     GTK_UPDATE_ALWAYS );
  gtk_signal_connect (GTK_OBJECT (mc->samples_adj), "value_changed",
		     GTK_SIGNAL_FUNC (adj_value_changed_cb), mc);

  /* Color selectors */
  mc->background_sel =
    GNOME_COLOR_PICKER (gnome_color_picker_new ());

  gtk_signal_connect (GTK_OBJECT (mc->background_sel), "color_set",
		     GTK_SIGNAL_FUNC (col_value_changed_cb), mc);

  // FIXME: This is a remnant from when the fire applet cared about a
  // background color.  It should be removed.
  r = g = b = 42;
  
  gnome_color_picker_set_i8 (mc->background_sel, r, g, b, 255);

  l = gtk_label_new (_("Background:"));
  gtk_table_attach (GTK_TABLE (t), l, 0, 1, 4, 5, 0, 0, 0, 0);
  gtk_table_attach (GTK_TABLE (t),
	    GTK_WIDGET (mc->background_sel), 1, 2, 4, 5, GTK_EXPAND, 0, 0, 0);

  /** Standard things */
  gtk_signal_connect (GTK_OBJECT (mc->prop_win), "destroy",
		      GTK_SIGNAL_FUNC (prop_cancel), mc);
	
  gtk_signal_connect (GTK_OBJECT (mc->prop_win), "delete_event",
		      GTK_SIGNAL_FUNC (prop_cancel), mc);
	
  gtk_signal_connect (GTK_OBJECT (mc->prop_win), "apply",
		      GTK_SIGNAL_FUNC (prop_apply), mc);

  gtk_signal_connect (GTK_OBJECT (mc->prop_win), "help",
		      GTK_SIGNAL_FUNC (gnome_help_pbox_display),
		      &help_entry);
	
  gtk_widget_show_all (GTK_WIDGET (mc->prop_win));

} /* bubblemon_properties_window */


/*
 *
 * Properties window button callbacks (apply, cancel, etc)
 *
 */
static int
prop_cancel (GtkWidget * w, gpointer data)
{
  BubbleMonData * mc = data;

  return FALSE;
} /* prop_cancel */

static void
prop_apply (GtkWidget *w, int page, gpointer data)
{
  BubbleMonData * mc = data;
  int breadth, depth, update, samples;
  guint8 r, g, b;

  /*
   * Update the running session from the properties.  The session
   * state will be saved when the applet exits and the panel tells it
   * to save state.
   */

  /*
   * Don't let the update function run while we're changing these
   * values.
   */
  mc->setup = FALSE;

/*  gtk_timeout_remove (mc->update_timeout_id);
  mc->update_timeout_id = gtk_timeout_add (1000 * mc->update_interval,
					   (GtkFunction) bubblemon_update, mc); */

  /*
   * Update the size
   */
  breadth = GTK_ADJUSTMENT (mc->breadth_adj)->value;
  depth = GTK_ADJUSTMENT (mc->depth_adj)->value;

  if ((breadth != mc->breadth) || (depth != mc->depth)) {
    mc->breadth = breadth;
    mc->depth = depth;
    bubblemon_set_size (mc);
  }

  update = GTK_ADJUSTMENT (mc->update_adj)->value;
  if (update != mc->update) {
    mc->update = update;
    bubblemon_set_timeout (mc);
  }
  
  samples = GTK_ADJUSTMENT (mc->samples_adj)->value;
  if (samples != mc->samples) {
    mc->samples = samples;
    bubblemon_setup_samples (mc);
  }
  
  gnome_color_picker_get_i8 ( mc->background_sel, &r, &g, &b, NULL);

  bubblemon_setup_colours (mc);

  mc->setup = TRUE;

  /* Make the panel save our config */
  applet_widget_sync_config (APPLET_WIDGET (mc->applet));
} /* prop_apply */

/*
 *
 * Property element callbacks (whenever a property is changed one of
 * these is called)
 *
 */

void
adj_value_changed_cb ( GtkAdjustment * ignored, gpointer data )
{
  BubbleMonData * mc = data;

  gnome_property_box_changed (GNOME_PROPERTY_BOX (mc->prop_win)); 
} /* value_changed_cb */

void
col_value_changed_cb ( GtkObject * ignored, guint arg1, guint arg2,
		      guint arg3, guint arg4, gpointer data )
{
  BubbleMonData * mc = data;

  gnome_property_box_changed (GNOME_PROPERTY_BOX (mc->prop_win)); 
} /* col_value_changed_cb */
