## Process this file with automake to produce Makefile.in

INCLUDES = \
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = bubblemon_applet

bubblemon_applet_SOURCES = \
	bubblemon.c bubblemon.h \
	session.c session.h

bubblemon_applet_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS) \
	$(GNOME_APPLETS_LIBS) \
	$(LIBGTOP_LIBS) \
	$(LIBGTOP_EXTRA_LIBS)
