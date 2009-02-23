## Process this file with automake to produce Makefile.in

localedir=$(datadir)/locale
defaultincludes = -I. -I$(srcdir) -I..

INCLUDES = \
	-I $(defaultincludes) \
	-DGTK_ENABLE_BROKEN \
	-DGNOMELOCALEDIR=\"$(prefix)/$(DATADIRNAME)/locale\" \
	$(GNOME_APPLETS_CFLAGS) \
	$(GTOP_APPLETS_CFLAGS)

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

libexec_PROGRAMS = bubblemon-gnome2

bubblemon_gnome2_SOURCES = \
	bubblemon.c bubblemon.h \
	gnome2-ui.c gnome2-ui.h \
	meter.h ui.h \
	gnome1-meter.c \
	unix-mail.c mail.h \
	ackumulator.c ackumulator.h

bubblemon_gnome2_LDADD = \
	$(GNOME_APPLETS_LIBS)   \
	$(GTOP_APPLETS_LIBS)

EXTRA_DIST = msgInBottle.c

