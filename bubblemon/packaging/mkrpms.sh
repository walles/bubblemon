#!/bin/sh

# This script attempts to build .deb, .rpm and .tar.gz packages from
# the bubblemon.

# FIXME: This script is extremely non-portable and is only guaranteed
# to work at Johan's place.  Portability enhancements are greatly
# appreciated, send them to d92-jwa@nada.kth.se.

# FIXME: This script will erase files in your RPM_SOURCES directory,
# in your BUBBLEMON_ROOT directory and in your RPMS directory.  For
# this script to ever get any good it should probably be rewritten in
# some real scripting language.

# FIXME: If you don't have write permissions in your RPM_SOURCES
# directory, in your BUBBLEMON_ROOT directory or in your RPMS
# directory, I don't know what happens.

BUBBLEMON_ROOT=/home/rael/bubbletest
RPM_SOURCES=/usr/src/redhat/SOURCES
RPM_SRPMS=/usr/src/redhat/SRPMS

# See to that we are in the root bubblemon directory
if [ `pwd` != $BUBBLEMON_ROOT ] ; then
    cat <<EOF
This script ($0) works only when started from the directory in the
\$BUBBLEMON_ROOT variable (currently $BUBBLEMON_ROOT).

The \$BUBBLEMON_ROOT variable can be configured at the top of this
script.  It must *not* end with a slash (/).
EOF

    exit 1
fi

if [ ! -d $RPM_SOURCES ] ; then 
cat <<EOF
This script ($0) needs of the $RPM_SOURCES directory to work.
Please fix this problem and try again.
EOF
    exit 1
fi

if [ ! -d $RPM_SRPMS ] ; then 
cat <<EOF
This script ($0) needs of the $RPM_SRPMS directory to work.
Please fix this problem and try again.
EOF
    exit 1
fi

# Build a source package
make -j2 dist
if [ $? != 0 ] ; then
    echo Error: Source package building failed > /dev/stderr
    exit 1
fi

# Build an RPM
rm -f bubblemon-*.tar.gz
make -j2 dist
if [ $? != 0 ] ; then
    echo Error: Source package building failed > /dev/stderr
    exit 1
fi
ln -s $BUBBLEMON_ROOT/bubblemon-*.tar.gz $RPM_SOURCES
rpm -bs packaging/bubblemon.spec
if [ $? != 0 ] ; then
    echo Error: RPM package building failed > /dev/stderr
    rm -f bubblemon-*.tar.gz $RPM_SOURCES/bubblemon-*.tar.gz
    exit 1
fi

echo
echo Package creation done, look in $RPM_SRPMS for the generated packages.
echo
