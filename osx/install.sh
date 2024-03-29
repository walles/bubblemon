#!/bin/bash

set -eufo pipefail

echo "INFO: Checking build environment..."

# Verify we're on Darwin
if [ "$(uname)" != "Darwin" ]; then
    echo >&2 "ERROR: Bubblemon only works on macOS"
    exit 1
fi

# Verify XCode is installed
XCODEDIR="/Applications/Xcode.app/Contents/Developer"
if [ ! -d "$XCODEDIR" ]; then
    echo >&2 "ERROR: Need XCode to build Bubblemon. Install XCode from here then try again: "
    echo >&2 "  https://apps.apple.com/se/app/xcode/id497799835?mt=12"

    exit
fi

if [ "$(xcode-select --print-path)" != "$XCODEDIR" ]; then
    # https://github.com/walles/bubblemon/issues/11
    echo
    echo "Picking a complete XCode installation to build from, please enter your password when prompted:"
    echo
    echo "\$ sudo xcode-select -s $XCODEDIR"
    sudo xcode-select -s "$XCODEDIR"
fi

# Verify we can xcrun git and xcodebuild
if xcrun git --version >/dev/null && xcrun xcodebuild -version >/dev/null; then
    echo "INFO: Command line dev tools found, proceeding..."
elif xcode-select --install; then
    echo >&2 "WARNING: Developer tools not found, trying to get them installed."
    echo >&2
    echo >&2 "After they are in place, run this script again!"
    echo >&2
    echo >&2 "If the automatic install doesn't work, check here:"
    echo >&2 "https://osxdaily.com/2014/02/12/install-command-line-tools-mac-os-x/"

    exit
else
    # xcode-select failed: https://stackoverflow.com/a/47804075/473672
    echo >&2
    echo >&2 "ERROR: Installing development tools failed. Try this manually:"
    echo >&2
    echo >&2 "  sudo rm -rf /Library/Developer/CommandLineTools && xcode-select --install"
    echo >&2
    echo >&2 "This will remove your current development tools and reinstall them."
    echo >&2 "Source: <https://stackoverflow.com/a/47804075/473672>"
    echo >&2
    echo >&2 "Then run this script again."

    exit 1
fi

MYDIR="$(
    cd "$(dirname "$0")"
    pwd
)"
if [ -d "$MYDIR/bubblemon.xcodeproj" ]; then
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
echo "INFO: Now building, this can take 30+ seconds and a lot of text can scroll by..."
date
TARGET_BUILD_DIR=$(mktemp -d -t bubblemon-build)
TARGET_TEMP_DIR=$(mktemp -d -t bubblemon-temp)
time xcrun xcodebuild \
    build \
    -quiet \
    -project osx/bubblemon.xcodeproj \
    -configuration Release \
    -target "Bubblemon" \
    -target "Bubblemon TouchBar" \
    -target "Bubblemon Menu Bar" \
    CONFIGURATION_BUILD_DIR="${TARGET_BUILD_DIR}" \
    CONFIGURATION_TEMP_DIR="${TARGET_TEMP_DIR}"

echo "INFO: Build done, now installing..."

BUBBLEMON_APP="$TARGET_BUILD_DIR/Bubblemon.app"
BUBBLEMON_TOUCHBAR_APP="$TARGET_BUILD_DIR/Bubblemon TouchBar.app"
BUBBLEMON_MENU_BAR_APP="$TARGET_BUILD_DIR/Bubblemon Menu Bar.app"

# Back up any existing installation
if [ -e "/Applications/Bubblemon.app" ]; then
    rm -rf "/Applications/Bubblemon.app.old"
    mv "/Applications/Bubblemon.app" "/Applications/Bubblemon.app.old"
fi
if [ -e "/Applications/Bubblemon TouchBar.app" ]; then
    rm -rf "/Applications/Bubblemon TouchBar.app.old"
    mv "/Applications/Bubblemon TouchBar.app" "/Applications/Bubblemon TouchBar.app.old"
fi
if [ -e "/Applications/Bubblemon Menu Bar.app" ]; then
    rm -rf "/Applications/Bubblemon Menu Bar.app.old"
    mv "/Applications/Bubblemon Menu Bar.app" "/Applications/Bubblemon Menu Bar.app.old"
fi

# Install!
mv "$BUBBLEMON_APP" "/Applications/"
mv "$BUBBLEMON_TOUCHBAR_APP" "/Applications/"
mv "$BUBBLEMON_MENU_BAR_APP" "/Applications/"

# Start the menu bar app
echo "INFO: Now installing the Menu Bar app..."
open "/Applications/Bubblemon Menu Bar.app"

# Invoke the TouchBar flavor from /Applications. If we don't have any TouchBar,
# the app should notice and not do anything.
echo
echo "INFO: Now installing the TouchBar app..."
open "/Applications/Bubblemon TouchBar.app" --args --reinstall

# Invoke the Dockapp flavor from /Applications. This will restart the Dock,
# which can be a jarring experience if you are not prepared for it, must be
# documented in the install instructions.
echo
echo "INFO: Now installing the Dock app..."
echo "WARNING: Your desktop will restart, this is fine!"
open "/Applications/Bubblemon.app" --args --reinstall

echo
echo "INFO: All done, you can now close this window and enjoy your bubbles."
echo
echo "Legend available at <https://walles.github.io/bubblemon/>."
