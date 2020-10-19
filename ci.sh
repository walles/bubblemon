#!/bin/bash

# Invoked through .travis.yml, should work fine locally as well.
#
# Runs CI testing.

set -eufo pipefail

MYDIR="$(cd "$(dirname "$0")"; pwd)"
cd "$MYDIR"

OUTPUT=$(mktemp -t bubblemon-analyze)

SCHEMES=("Debug" "Bubblemon TouchBar" "Bubblemon TouchBar Helper")
for scheme in "${SCHEMES[@]}" ; do
    echo "Analyzing scheme: $scheme"
    xcodebuild -project osx/bubblemon.xcodeproj/ -scheme "$scheme" clean analyze | tee "$OUTPUT"

    # Latest(ish?) xcodebuild October 2020 didn't fail with an exit code on
    # analysis problems, this is an attempt at handling that.
    grep -A10 "warning: " "$OUTPUT" && exit 1
done
