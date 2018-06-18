#! /bin/bash

set -e
set -x

# use RAM disk if possible
if [ "$CI" == "" ] && [ -d /dev/shm ]; then
    TEMP_BASE=/dev/shm
else
    TEMP_BASE=/tmp
fi

BUILD_DIR=$(mktemp -d -p "$TEMP_BASE" AppImageUpdate-build-XXXXXX)

cleanup () {
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
}

trap cleanup EXIT

# store repo root as variable
REPO_ROOT=$(readlink -f $(dirname $(dirname $0)))
OLD_CWD=$(readlink -f .)

pushd "$BUILD_DIR"

cmake "$REPO_ROOT" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBLACKMAGIC_SDK_DIR="${BLACKMAGIC_SDK_DIR:-~/Blackmagic\ DeckLink\ SDK\ 10.10/Linux/}"

make -j$(nproc)

wget https://github.com/TheAssassin/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage
./linuxdeploy-x86_64.AppImage --appimage-extract
mv squashfs-root/ linuxdeploy

linuxdeploy/AppRun --appdir AppDir --init-appdir -e decklink-debugger -d "$REPO_ROOT"/resources/decklink-debugger.desktop -i "$REPO_ROOT"/resources/decklink-debugger.svg

wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage
./appimagetool-x86_64.AppImage --appimage-extract
mv squashfs-root/ appimagetool

appimagetool/AppRun AppDir

mv decklink-debugger*.AppImage "$OLD_CWD"

