#!/bin/sh

# This script attempts to build .rpm packages from
# the bubblemon.

# Is totaly unportable. 

CURRENT_VERSION=bubblemon-2.0.0.tar.gz
RPM_SOURCES=/usr/src/redhat/SOURCES
RPM_SRPMS=/usr/src/redhat/SRPMS

if [ ! -f $RPM_SOURCES/$CURRENT_VERSION ] ; then 
cat <<EOF
This script ($0) needs that the tar.gz exists in the $RPM_SOURCES directory to work.
Please fix this problem and try again.
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

rpm -bs bubblemon.spec
if [ $? != 0 ] ; then
    echo Error: RPM package building failed > /dev/stderr
    exit 1
fi

echo
echo Package creation done, look in $RPM_SRPMS for the generated packages.
echo
