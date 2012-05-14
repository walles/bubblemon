## Gnome3 stuff
APPLET_LOCATION = $(libexecdir)/bubblemon-gnome3

appletdir       = @PANEL_APPLET_DIR@
applet_in_files = org.gnome.panel.BubblemonApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files:applet.in=applet.in.in)
	$(AM_V_GEN)sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" $< > $@

uidir = $(datadir)/gnome-panel/ui
ui_DATA = bubblemon-menu.xml

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po); $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       = @DBUS_SERVICE_DIR@
service_in_files = org.gnome.panel.applet.BubblemonAppletFactory.service.in
service_DATA     = $(service_in_files:.service.in=.service)

org.gnome.panel.applet.BubblemonAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
            -e "s|\@LOCATION\@|$(APPLET_LOCATION)|" \
            $< > $@

EXTRA_DIST = \
	$(applet_in_files).in \
        $(service_in_files) \
	$(ui_DATA)

CLEANFILES = \
	$(applet_in_files) $(applet_DATA) $(service_DATA)
