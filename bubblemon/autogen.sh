#!/bin/sh

# Make sure we are in the script directory
cd $(dirname $0)

echo "Generating configuration files for bubblemon..."
echo

aclocal-1.7 &&\
autoheader2.50 &&\
automake-1.7 --add-missing &&\
autoconf2.50 ||\ exit $?

echo
echo "Done generating configuration files for bubblemon, now do \"./configure ; make ; make install\""
