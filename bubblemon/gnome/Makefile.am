## Gnome2 stuff

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_BubblemonApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = GNOME_BubblemonApplet.xml

EXTRA_DIST = $(server_DATA) \
	$(ui_DATA)
