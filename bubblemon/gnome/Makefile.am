## Gnome2 stuff

SUBDIRS = 

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_BubblemonApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_BubblemonApplet.xml

@INTLTOOL_SERVER_RULE@

schemasdir       = $(sysconfdir)/gconf/schemas
schemas_in_files = bubblemon.schemas.in
schemas_DATA     = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = \
	GNOME_BubblemonApplet.server.in.in 	\
	$(schemas_in_files)             	\
	$(schemas_DATA)				\
	$(ui_DATA)


install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$$p ; \
		done \
	fi

