#! /bin/sh

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

BUBBLEMON_ROOT=/home/rael/src/Gnome/bubblemon
RPM_SOURCES=/home/rael/src/RPM/SOURCES
RPMS=/home/rael/src/RPM

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

# Build a Debian package
debuild -uc -us
if [ $? != 0 ] ; then
    echo Error: Debian package building failed > /dev/stderr
    exit 1
fi
rm ../*.changes ../*.dsc ../*.dsc.asc ../bubblemon_*.tar.gz

# Build a source package
make -j2 dist
if [ $? != 0 ] ; then
    echo Error: Source package building failed > /dev/stderr
    exit 1
fi
mv bubblemon-*.tar.gz ..

# Build an RPM
rm -f bubblemon-*.tar.gz
make -j2 dist
if [ $? != 0 ] ; then
    echo Error: Source package building failed > /dev/stderr
    exit 1
fi
ln -s $BUBBLEMON_ROOT/bubblemon-*.tar.gz $RPM_SOURCES
rm -f $RPMS/RPM/*/bubblemon*rpm $RPMS/SRPM/bubblemon*rpm
rpm -bs packaging/bubblemon.spec
if [ $? != 0 ] ; then
    echo Error: RPM package building failed > /dev/stderr
    rm -f bubblemon-*.tar.gz $RPM_SOURCES/bubblemon-*.tar.gz
    exit 1
fi
echo Note that the following command may fail, but that is OK
mv $RPMS/RPMS/*/bubblemon*rpm $RPMS/SRPMS/bubblemon*rpm ..
rm bubblemon-*.tar.gz $RPM_SOURCES/bubblemon-*.tar.gz

echo
echo Package creation done, look in .. for the generated packages.
echo
