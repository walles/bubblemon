/*
 * licencia
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include <glade/glade.h>

#include "gnome2-ui.h"
#include "bubblemon-prefs-dialog.h"

static void
netload_toggled (GtkToggleButton *button, gpointer data)
{
GConfClient *client;
gboolean     value;

  client = gconf_client_get_default ();
  value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
  gconf_client_set_bool (client, GC_KEY_NETLOAD, value, NULL);
}

static void
mailcheck_toggled (GtkToggleButton *button, gpointer data)
{
GConfClient *client;
gboolean     value;

  client = gconf_client_get_default ();
  value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
  gconf_client_set_bool (client, GC_KEY_MAILCHECK, value, NULL);
}

static void
response_cb (GtkDialog *dialog, gint id, gpointer data)
{
BubblemonApplet *bubble = data;

  gtk_widget_hide (GTK_WIDGET (bubble->pref.preferences_dialog));
}

void
bubblemon_prefs_run (BubblemonApplet *bubble)
{
  GladeXML  *glade_xml;
  
  if (bubble->pref.preferences_dialog) {
  	gtk_window_present (GTK_WINDOW (bubble->pref.preferences_dialog));
  return;
  }

  glade_xml = glade_xml_new (GLADE_DIR"/bubblemon_pref_dialog.glade",
                               "bubblemon_preferences", NULL);

  bubble->pref.preferences_dialog= glade_xml_get_widget (glade_xml, "bubblemon_preferences");

  bubble->pref.w_netload = glade_xml_get_widget (glade_xml, "netload_toggle");
  bubble->pref.w_mailcheck = glade_xml_get_widget (glade_xml, "mailcheck_toggle");

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bubble->pref.w_netload), bubble->pref.gnetload);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bubble->pref.w_mailcheck), bubble->pref.gmailcheck);

  g_signal_connect (G_OBJECT (bubble->pref.w_netload), "toggled", G_CALLBACK (netload_toggled), bubble);
  g_signal_connect (G_OBJECT (bubble->pref.w_mailcheck), "toggled", G_CALLBACK (mailcheck_toggled), bubble);
			
  g_signal_connect (G_OBJECT (bubble->pref.preferences_dialog), "response", G_CALLBACK (response_cb), bubble);
  gtk_widget_show_all (GTK_WIDGET (bubble->pref.preferences_dialog));
			      
}

void update_bubblemon_pref_windows(BubblemonApplet *bubble)
{
  if (bubble->pref.preferences_dialog) {
     gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bubble->pref.w_netload), bubble->pref.gnetload);
     gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bubble->pref.w_mailcheck), bubble->pref.gmailcheck);
  }
}
