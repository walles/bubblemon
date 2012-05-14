## Process this file with automake to produce Makefile.in

localedir=$(datadir)/locale
xmluidir=$(datadir)/gnome-panel/ui

INCLUDES = \
	-DGTK_ENABLE_BROKEN \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	-DBUBBLEMON_MENU_UI_DIR=\"$(xmluidir)\" \
	$(PLATFORM_CFLAGS)

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

# bubblemon-program is a placeholder name; changed by install-exec-hook below 
libexec_PROGRAMS = bubblemon-program

bubblemon_program_SOURCES = \
	bubblemon.c bubblemon.h \
	meter.h ui.h \
	gnome1-meter.c \
	unix-mail.c mail.h \
	accumulator.c accumulator.h

# can't use BUBBLEMON_PLATFORM when setting bubblemon_program_SOURCES, so..
if GNOME3_BUILD
bubblemon_program_SOURCES += gnome3-ui.c gnome3-ui.h
endif
if GNOME2_BUILD
bubblemon_program_SOURCES += gnome2-ui.c gnome2-ui.h
endif
if XFCE4_BUILD
bubblemon_program_SOURCES += xfce4-ui.c xfce4-ui.h
endif

bubblemon_program_LDADD = \
	$(PLATFORM_LIBS)

EXTRA_DIST = msgInBottle.c

# rename the program into its platform-based equivalent
install-exec-hook:
	cd $(DESTDIR)$(libexecdir) && \
	mv -f bubblemon-program$(EXEEXT) bubblemon-$(BUBBLEMON_PLATFORM)$(EXEEXT)
