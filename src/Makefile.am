## Process this file with automake to produce Makefile.in

localedir=$(datadir)/locale
plugindir = $(libexecdir)/xfce4/panel-plugins

# XXX how to swicth here depending on configure? 
plugin_PROGRAMS = xfce4-bubblemon-plugin
#libexec_PROGRAMS = bubblemon-gnome2

xfce4_bubblemon_plugin_CFLAGS =					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"			\
	$(LIBGTOP_CFLAGS)					\
	$(LIBXFCEGUI4_CFLAGS)					\
	$(LIBXFCE4PANEL_CFLAGS)

INCLUDES = 							\
	-DGTK_ENABLE_BROKEN 					\
	-DGNOMELOCALEDIR=\"$(prefix)/$(DATADIRNAME)/locale\"

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

bubblemon_gnome2_CFLAGS = 					\
	$(BUBBLEMON_CFLAGS)

bubblemon_gnome2_SOURCES = \
	bubblemon.c bubblemon.h \
	gnome2-ui.c gnome2-ui.h \
	meter.h ui.h \
	gnome1-meter.c \
	unix-mail.c mail.h \
	accumulator.c accumulator.h

xfce4_bubblemon_plugin_SOURCES = \
	bubblemon.c bubblemon.h \
	xfce4-ui.c xfce4-ui.h \
	meter.h ui.h \
	gnome1-meter.c \
	unix-mail.c mail.h \
	accumulator.c accumulator.h

bubblemon_gnome2_LDADD =					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"			\
	$(BUBBLEMON_LIBS)

xfce4_bubblemon_plugin_LDADD =					\
	$(LIBGTOP_LIBS)						\
	$(LIBXFCEGUI4_LIBS)					\
	$(LIBXFCE4PANEL_LIBS)

EXTRA_DIST = msgInBottle.c
