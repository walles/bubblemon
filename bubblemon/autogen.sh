#!/bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="The Bubbling Load Monitoring Applet"

(test -f $srcdir/configure.in \
		&& test -f $srcdir/ChangeLog \
		&& test -d $srcdir/src) || {
	echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
	echo " top-level $PKG_NAME directory"
	exit 1
}

which gnome-autogen.sh || {
	echo "You need to install gnome-common from the GNOME CVS"
	exit 1
}
USE_GNOME2_MACROS=1 . gnome-autogen.sh

			         
(automake --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have automake installed to compile bubblemon"
	echo
	exit
}

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have autoconf installed to compile bubblemon"
	echo
	exit
}

echo "Generating configuration files for bubblemon..."
echo

aclocal
autoheader
automake --add-missing
autoconf

echo
echo "Done generating configuration files for bubblemon, now do \"./configure ; make ; make install\""
