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

# FIXME: Create a working directory
# FIXME: cd into working directory
# FIXME: git clone bubblemon into that working directory

# FIXME: Build both the Dock flavor and the TouchBar flavor

# FIXME: Install both Dock flavor and TouchBar flavor into /Applications, after
# first backing up any version already in place there

# FIXME: Invoke the Dockapp flavor from /Applications

# FIXME: If we have a touchBar, invoke the TouchBar flavor from /Applications
