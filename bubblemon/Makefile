# yes, I know this is the weakest Makefile ever, but it is sufficient
# for my purposes; i.e., writing and releasing this in a day.

mode = debug

prefix = /usr/local
bindir = ${prefix}/bin
sharedir = ${prefix}/share
etcdir = ${prefix}/etc

ccFlags_release = -O3
ccFlags_debug = -g
ccFlags = ${ccFlags_${mode}} -c -Wall -W -Wno-unused

cppFlags = ${shell gnome-config --cflags glib gnome gnomeui gnorba}

ldFlags_release = -s
ldFlags_debug = -g
ldFlags = ${ldFlags_${mode}}

ldLibs = ${shell gnome-config --libs glib gnome gnomeui gnorba} -lpanel_applet -lgtop -lgtop_sysdeps -lgtop_common -lXau

srcs = bubblemon.c properties.c session.c

objs = ${srcs:.c=.o}

target = bubblemon_applet

.PHONY: clean

${target}: ${objs}
	gcc ${ldFlags} $^ ${ldLibs} -o $@

%.o: %.c
	gcc ${ccFlags} ${cppFlags} $< -o $@

clean:
	rm -f ${objs} ${target}

install: ${target}
	install -c -D -m755 ${target} ${bindir}/${target}
	install -c -D -m644 ${target}.gnorba ${etcdir}/CORBA/servers/${target}.gnorba
	install -c -D -m644 ${target}.desktop ${sharedir}/applets/Monitors/${target}.desktop
