#!/bin/bash

# Set Version string from git describe
#
# See: https://developer.apple.com/library/content/documentation/General/Reference/InfoPlistKeyReference/Articles/CoreFoundationKeys.html

set -e
set -x

INFO_PLIST="${TARGET_BUILD_DIR}/${INFOPLIST_PATH}"

VERSION=$(git describe --match='osx-*' --dirty | cut -d- -f2)
/usr/libexec/PlistBuddy -c "Add :CFBundleShortVersionString string \"$VERSION\"" "${INFO_PLIST}" ||
/usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString \"$VERSION\"" "${INFO_PLIST}"

/usr/libexec/PlistBuddy -c "Add :CFBundleVersion string \"$VERSION\"" "${INFO_PLIST}" ||
/usr/libexec/PlistBuddy -c "Set :CFBundleVersion \"$VERSION\"" "${INFO_PLIST}"
