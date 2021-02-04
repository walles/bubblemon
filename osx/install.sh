#!/bin/bash

set -eufo pipefail

# Verify we're on Darwin
if [ "$(uname)" != "Darwin" ] ; then
    >&2 echo "ERROR: Bubblemon only works on macOS"
    exit 1
fi

# Verify we can xcrun git and xcodebuild
if xcrun git --version > /dev/null && xcrun xcodebuild -version > /dev/null; then
    echo "INFO: Command line dev tools found, proceeding..."
else
    >&2 echo "WARNING: Developer tools not found, trying to get them installed."
    >&2 echo
    >&2 echo "After they are in place, try running this script again!"
    >&2 echo
    >&2 echo "If the automatic install doesn't work, check here:"
    >&2 echo "https://osxdaily.com/2014/02/12/install-command-line-tools-mac-os-x/"
    exit
fi

# Create a working directory
WORKDIR="$(mktemp -d -t bubblemon)"
cd "$WORKDIR"

# git clone bubblemon into that working directory
xcrun git clone "https://github.com/walles/bubblemon.git"
cd bubblemon

# Build both the Dock flavor and the TouchBar flavor
echo "Now building, this can take 10+ seconds..."
TARGET_BUILD_DIR=$(mktemp -d)
TARGET_TEMP_DIR=$(mktemp -d)
xcodebuild \
  build \
  -quiet \
  -project osx/bubblemon.xcodeproj \
  -configuration Release \
  -target "Bubblemon" \
  -target "Bubblemon TouchBar" \
  CONFIGURATION_BUILD_DIR="${TARGET_BUILD_DIR}" \
  CONFIGURATION_TEMP_DIR="${TARGET_TEMP_DIR}"

BUBBLEMON_APP="$TARGET_BUILD_DIR/Bubblemon.app"
BUBBLEMON_TOUCHBAR_APP="$TARGET_BUILD_DIR/Bubblemon TouchBar.app"

# Back up any existing installation
if [ -e "/Applications/Bubblemon.app" ] ; then
    rm -rf "/Applications/Bubblemon.app.old"
    mv "/Applications/Bubblemon.app" "/Applications/Bubblemon.app.old"
fi
if [ -e "/Applications/Bubblemon TouchBar.app" ] ; then
    rm -rf "/Applications/Bubblemon TouchBar.app.old"
    mv "/Applications/Bubblemon TouchBar.app" "/Applications/Bubblemon TouchBar.app.old"
fi

# Install!
mv "$BUBBLEMON_APP" "/Applications/"
mv "$BUBBLEMON_TOUCHBAR_APP" "/Applications/"

# FIXME: Invoke the Dockapp flavor from /Applications

# FIXME: If we have a touchBar, invoke the TouchBar flavor from /Applications
