## Process this file with automake to produce Makefile.in

@SET_MAKE@

man_MANS	= bubblemon-program.1
mandir		= @mandir@/sv

EXTRA_DIST	= bubblemon-program.1.in

install-data-hook:
	cd $(DESTDIR)$(man1dir) && \
	mv -f bubblemon-program.1 bubblemon-$(BUBBLEMON_PLATFORM).1
