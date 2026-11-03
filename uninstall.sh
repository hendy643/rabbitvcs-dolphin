#!/bin/sh
# Remove the RabbitVCS Dolphin VCS plugins installed by install.sh.
set -eu

cd "$(dirname "$0")"

MANIFEST="build/install_manifest.txt"
if [ ! -f "$MANIFEST" ]; then
    echo "No $MANIFEST found; run ./install.sh first." >&2
    exit 1
fi

PREFIX=/usr
if [ -f build/CMakeCache.txt ]; then
    PREFIX=$(sed -n 's/^CMAKE_INSTALL_PREFIX:PATH=//p' build/CMakeCache.txt)
fi

if [ "$(id -u)" -eq 0 ] || [ -w "$PREFIX" ]; then
    while IFS= read -r file || [ -n "$file" ]; do
        if [ -e "$file" ]; then
            rm -f -- "$file"
        fi
    done < "$MANIFEST"
else
    sudo sh -c '
        while IFS= read -r file || [ -n "$file" ]; do
            if [ -e "$file" ]; then
                rm -f -- "$file"
            fi
        done
        exit 0
    ' < "$MANIFEST"
fi
