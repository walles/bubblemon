#!/bin/bash

# Create a .zip in ~/Downloads for release builds

set -e
set -o pipefail
set -u

MYDIR=$(dirname "$0")
MYDIR=$(cd "$MYDIR" ; pwd)

# This will both build and analyze, see the xcodebuild man page
XCODE_ACTION="analyze"

TARGET_BUILD_DIR=$(mktemp -d)
TARGET_TEMP_DIR=$(mktemp -d)

xcodebuild \
  ${XCODE_ACTION} \
  -project "${MYDIR}"/bubblemon.xcodeproj \
  -configuration Release \
  -target Bubblemon \
  -target "Bubblemon TouchBar" \
  CONFIGURATION_BUILD_DIR="${TARGET_BUILD_DIR}" \
  CONFIGURATION_TEMP_DIR="${TARGET_TEMP_DIR}"

VERSION_NAME=$(cd "$MYDIR"; git describe --match='osx-*' --dirty | sed 's/^osx-//')
TAG="bubblemon-osx-${VERSION_NAME}"
ZIPDIR="$(pwd)"
ZIPNAME="${TAG}.zip"

# The symlinking is to make unpacking bubblemon-osx-1234.zip create
# bubblemon-osx-1234/Bubblemon.app rather than just Bubblemon.app.
# We assume Info-ZIP here which treats the directory symlinks as if
# it was just a directory.
rm -f "${ZIPDIR}/${ZIPNAME}" "${TARGET_BUILD_DIR}/${TAG}"
ln -s "${TARGET_BUILD_DIR}" "${TARGET_BUILD_DIR}/${TAG}"
(cd "${TARGET_BUILD_DIR}"; zip -r "${ZIPDIR}/${ZIPNAME}" "${TAG}/Bubblemon.app" "${TAG}/Bubblemon TouchBar.app")

rm -rf "${TARGET_BUILD_DIR}" "${TARGET_TEMP_DIR}"

echo
echo "Release archive written into ${ZIPNAME}"
