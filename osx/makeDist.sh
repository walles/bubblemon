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
TAG="bubblemon-touchbar-${VERSION_NAME}"
ZIPNAME="${TAG}.zip"

DISTDIR="$(cd "${MYDIR}"/..; pwd)"/dist
rm -rf "$DISTDIR"
mkdir -p "$DISTDIR"

# The symlinking is to make unpacking bubblemon-osx-1234.zip create
# bubblemon-osx-1234/Bubblemon.app rather than just Bubblemon.app.
# We assume Info-ZIP here which treats the directory symlinks as if
# it was just a directory.
rm -f "${DISTDIR}/${ZIPNAME}" "${TARGET_BUILD_DIR}/${TAG}"
ln -s "${TARGET_BUILD_DIR}" "${TARGET_BUILD_DIR}/${TAG}"
(cd "${TARGET_BUILD_DIR}"; zip -r "${DISTDIR}/${ZIPNAME}" "${TAG}/Bubblemon TouchBar.app")

cd "$MYDIR"
npm install --prefix ./build appdmg

cp -a "${TARGET_BUILD_DIR}/Bubblemon.app" "build"
./build/node_modules/.bin/appdmg "appdmg.json" "${DISTDIR}/Bubblemon-Dockapp-${VERSION_NAME}.dmg"

rm -rf "${TARGET_BUILD_DIR}" "${TARGET_TEMP_DIR}"

echo
echo "Release artifacts written into ${DISTDIR}:"
ls -l "${DISTDIR}"
