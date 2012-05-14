SUBDIRS = hu sv

man_MANS = bubblemon-program.1

EXTRA_DIST = bubblemon-program.1.in

install-data-hook:
	cd $(DESTDIR)$(mandir) && \
	mv -f bubblemon-program.1 bubblemon-$(BUBBLEMON_PLATFORM).1
