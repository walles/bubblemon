#!/bin/bash

set -eufo pipefail

echo "INFO: Checking build environment, this can take 10 seconds..."

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
    xcode-select --install
    exit
fi

MYDIR="$(cd "$(dirname "$0")"; pwd)"
if [ -d "$MYDIR/bubblemon.xcodeproj" ] ; then
    # We are already in the source tree, start from the top
    echo "INFO: Installing from: $MYDIR"
    cd "$MYDIR/.."
else
    # Clone Bubblemon into a temporary working directory
    WORKDIR="$(mktemp -d -t bubblemon)"
    cd "$WORKDIR"

    echo
    echo "INFO: Getting Bubblemon source code..."
    xcrun git clone "https://github.com/walles/bubblemon.git"
    cd bubblemon
fi

# Build both the Dock flavor and the TouchBar flavor
echo
echo "INFO: Now building, this can take 10+ seconds..."
TARGET_BUILD_DIR=$(mktemp -d -t bubblemon-build)
TARGET_TEMP_DIR=$(mktemp -d -t bubblemon-temp)
xcrun xcodebuild \
  build \
  -quiet \
  -project osx/bubblemon.xcodeproj \
  -configuration Release \
  -target "Bubblemon" \
  -target "Bubblemon TouchBar" \
  CONFIGURATION_BUILD_DIR="${TARGET_BUILD_DIR}" \
  CONFIGURATION_TEMP_DIR="${TARGET_TEMP_DIR}"

echo "INFO: Build done, now installing..."

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

# Invoke the Dockapp flavor from /Applications. This will restart the Dock,
# which can be a jarring experience if you are not prepared for it, must be
# documented in the install instructions.
echo "INFO: Now installing the Dock app, your screen might flicker..."
open "/Applications/Bubblemon.app"

# Invoke the TouchBar flavor from /Applications. If we don't have any TouchBar,
# the app should notice and not do anything.
echo "INFO: Now installing the TouchBar app..."
open "/Applications/Bubblemon TouchBar.app"

echo
echo "INFO: All done, you can now close this window and enjoy your bubbles."
echo
echo "Legend available at <https://walles.github.io/bubblemon/>."
