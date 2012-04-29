## Process this file with automake to produce Makefile.in

localedir=$(datadir)/locale

INCLUDES = \
	-DGTK_ENABLE_BROKEN \
	-DGNOMELOCALEDIR=\"$(prefix)/$(DATADIRNAME)/locale\" \
	$(BUBBLEMON_CFLAGS)

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

libexec_PROGRAMS = bubblemon-gnome2

bubblemon_gnome2_SOURCES = \
	bubblemon.c bubblemon.h \
	gnome2-ui.c gnome2-ui.h \
	meter.h ui.h \
	gnome1-meter.c \
	unix-mail.c mail.h \
	accumulator.c accumulator.h

bubblemon_gnome2_LDADD = \
	$(BUBBLEMON_LIBS)

EXTRA_DIST = msgInBottle.c

