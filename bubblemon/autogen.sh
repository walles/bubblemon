#!/bin/sh

# Make sure we are in the script directory
cd $(dirname $0)

echo "Generating configuration files for bubblemon..."
echo

aclocal &&\
autoheader &&\
automake --add-missing &&\
autoconf ||\ exit $?

echo
echo "Done generating configuration files for bubblemon, now do \"./configure ; make ; make install\""
