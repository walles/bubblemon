#!/bin/sh

(gettextize --version) < /dev/null > /dev/null 2>&1 || {
	echo "You must have gettext installed to compile bubblemon";
	exit
}

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

gettextize
aclocal -I macros
autoheader
automake --add-missing
autoconf

echo
echo "Done generating configuration files for bubblemon, now do \"./configure ; make ; make install\""
