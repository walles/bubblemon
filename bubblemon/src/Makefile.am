## Process this file with automake to produce Makefile.in

gladedir = $(datadir)/bubblemon
glade_DATA = bubblemon_pref_dialog.glade

localedir=$(datadir)/locale
defaultincludes = -I. -I$(srcdir) -I..

EGGFILES = \
	egg-screen-exec.h \
	egg-screen-exec.c \
	egg-screen-help.h \
	egg-screen-help.c \
	egg-screen-url.h  \
	egg-screen-url.c  \
	$(NULL)

INCLUDES = \
	-I $(defaultincludes) \
	-DGNOMELOCALEDIR=\"$(prefix)/$(DATADIRNAME)/locale\" \
	-DGLADE_DIR=\"$(gladedir)\" \
        -DPREFIX=\""$(prefix)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DLIBDIR=\""$(libdir)"\" \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	$(GNOME_APPLETS_CFLAGS) \
	$(LIBGLADE_CFLAGS) \
	$(GTOP_APPLETS_CFLAGS)

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

libexec_PROGRAMS = bubblemon-gnome2

bubblemon_gnome2_SOURCES = \
	bubblemon.c bubblemon.h \
	gnome2-ui.c gnome2-ui.h \
	meter.h ui.h \
	gnome1-meter.c \
	unix-mail.c mail.h \
	bubblemon-prefs-dialog.c bubblemon-prefs-dialog.h \
	bubblemon-about-dialog.c bubblemon-about-dialog.h \
	gnome1-netload.c netload.c netload.h \
	$(EGGFILES)

bubblemon_gnome2_LDADD = \
	$(GNOME_APPLETS_LIBS)   \
	$(LIBGLADE_LIBS) \
	$(GTOP_APPLETS_LIBS)

EXTRA_DIST = \
        $(glade_DATA)  \
	msgInBottle.c 

