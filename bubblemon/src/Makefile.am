## Process this file with automake to produce Makefile.in

EXTRA_DIST = msgInBottle.c

localedir=$(datadir)/locale
defaultincludes = -I. -I$(srcdir) -I..

INCLUDES = \
	$(GNOME_INCLUDEDIR) \
	-I $(LIBGTOP_INCLUDEDIR) \
	$(defaultincludes)
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS = bubblemon-gnome1

bubblemon_gnome1_SOURCES = \
	bubblemon.c bubblemon.h \
	meter.h ui.h \
	gnome1-meter.c gnome1-ui.c \
	unix-mail.c mail.h

bubblemon_gnome1_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOME_APPLETS_LIBS) \
	$(LIBGTOP_LIBS) \
	$(LIBGTOP_EXTRA_LIBS)
