## Process this file with automake to produce Makefile.in

localedir=$(datadir)/locale
defaultincludes = -I. -I$(srcdir) -I..

INCLUDES = \
	$(GNOME_INCLUDEDIR) \
	$(defaultincludes)
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS = bubblemon_applet

bubblemon_applet_SOURCES = \
	bubblemon.c bubblemon.h \
	session.c session.h

bubblemon_applet_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOME_APPLETS_LIBS) \
	$(LIBGTOP_LIBS) \
	$(LIBGTOP_EXTRA_LIBS)
