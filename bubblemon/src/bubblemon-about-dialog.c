/* licencia */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
															     #include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <gnome.h>
#include <panel-applet.h>

#include "gnome2-ui.h"
#include "bubblemon-about-dialog.h"

void
bubblemon_about_run (BubblemonApplet *bubble)
{
  static const gchar *authors[] = { "Johan Walles <d92-jwa@nada.kth.se>",
				    "Juan Salaverria <rael@vectorstar.net>",
				    NULL };
  static const gchar *documenters[] = { NULL };

  if (bubble->aboutbox != NULL) {
    gtk_window_present (GTK_WINDOW (bubble->aboutbox));
    return;
  }

  bubble->aboutbox= gnome_about_new(_("Bubbling Load Monitor"), VERSION,
				    _("Copyright (C) 1999-2002 Johan Walles"),
				    _("This applet displays your CPU load as a bubbling liquid.\n"
				      "This applet comes with ABSOLUTELY NO WARRANTY, "
				      "see the LICENSE file for details.\n"
				      "This is free software, and you are welcome to redistribute it "
				      "under certain conditions (GPL), "
				      "see the LICENSE file for details."),
				    authors,
				    documenters,
				    NULL,
				    NULL);
 
  gtk_window_set_wmclass (GTK_WINDOW (bubble->aboutbox), "bubblemon", "Bubblemon");

  g_signal_connect ( bubble->aboutbox, "destroy", G_CALLBACK (gtk_widget_destroyed), &bubble->aboutbox);
							    
  gtk_widget_show(bubble->aboutbox);

  return;
}
