#! /bin/sh

# This script attempts to build .deb, .rpm and .tar.gz packages from
# the bubblemon.

# FIXME: This script is extremely non-portable and is only guaranteed
# to work at Johan's place.  Portability enhancements are greatly
# appreciated, send them to d92-jwa@nada.kth.se.

BUBBLEMON_ROOT=/home/johan/src/Gnome/bubblemon
RPM_SOURCES=/home/johan/src/RPM/SOURCES
RPMS=/home/johan/src/RPM/RPMS/i386

# See to that we are in the root bubblemon directory
if [ `pwd` != $BUBBLEMON_ROOT ] ; then
    cat <<EOF
This script ($0) works only when started from the directory in the
\$BUBBLEMON_ROOT variable (currently $BUBBLEMON_ROOT).

The \$BUBBLEMON_ROOT variable can be configured at the top of this
script.
EOF

    exit 1
fi

# Build a Debian package
debuild
rm ../*.changes ../*.dsc ../*.dsc.asc ../bubblemon_*.tar.gz

# Build a source package
make dist
mv bubblemon-*.tar.gz ..

# Build an RPM
rm -f bubblemon-*.tar.gz
make dist
ln -s $(BUBBLEMON_ROOT)/bubblemon-*.tar.gz $(RPM_SOURCES)
rm -f $(RPMS)/bubblemon*rpm
rpm -bb packaging/bubblemon.spec
mv $(RPMS)/bubblemon*rpm ..
rm bubblemon-*.tar.gz
