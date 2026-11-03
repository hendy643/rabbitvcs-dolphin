#!/bin/sh
# Build the RabbitVCS Dolphin VCS plugins.
set -eu

cd "$(dirname "$0")/.."

PREFIX="${PREFIX:-/usr}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

cmake -S . -B build \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    "$@"
cmake --build build --parallel
