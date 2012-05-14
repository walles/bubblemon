## Process this file with automake to produce Makefile.in

@SET_MAKE@

man_MANS	= bubblemon-program.1
mandir		= @mandir@/hu

EXTRA_DIST	= bubblemon-program.1.in

install-data-hook:
	cd $(DESTDIR)$(mandir) && \
	mv -f bubblemon-program.1 bubblemon-$(BUBBLEMON_PLATFORM).1
