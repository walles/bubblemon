#!/bin/bash -e

# This script is a workaround for that xctool 0.2.1 does not emit
# helpful exit codes on analysis failures:
#
# https://github.com/facebook/xctool/issues/436

xctool -project osx/bubblemon.xcodeproj/ -scheme Debug clean

LOG=xctool.log
xctool -project osx/bubblemon.xcodeproj/ -scheme Debug -reporter plain:$LOG -reporter pretty analyze
if grep -qi 'warning:' $LOG ;then
    echo
    echo '****'
    echo '**** THERE WERE WARNINGS ****'
    echo '****'
    echo
    echo '**** See above for details'
    echo
    echo 'Misleading ANALYZE SUCCEEDED message reported here:'
    echo ' https://github.com/facebook/xctool/issues/436'
    exit 1
fi
