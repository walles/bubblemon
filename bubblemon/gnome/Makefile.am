## Gnome2 stuff

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_BubblemonApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_BubblemonApplet.xml

@INTLTOOL_SERVER_RULE@

EXTRA_DIST = \
	GNOME_BubblemonApplet.server.in.in \
	$(ui_DATA)

CLEANFILES = \
	$(server_in_files) \
	$(server_in_files:.in=)
