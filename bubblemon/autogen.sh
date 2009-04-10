#!/bin/sh

# Make sure we are in the script directory
cd $(dirname $0)

echo "Generating configuration files for bubblemon..."
echo

set -x

aclocal-1.10 -I m4 &&\
autoheader &&\
automake-1.10 --add-missing &&\
autoconf &&\
intltoolize --force || exit $?

set +x

echo
echo "Done generating configuration files for bubblemon, now do \"./configure && make && make install\""
