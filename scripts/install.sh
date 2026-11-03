#!/bin/sh
# Build and install the RabbitVCS Dolphin VCS plugins.
set -eu

cd "$(dirname "$0")/.."

PREFIX="${PREFIX:-/usr}"
export PREFIX

./scripts/build.sh

# Find the nearest existing directory to decide whether sudo is needed.
target="$PREFIX"
while [ ! -e "$target" ]; do
    target=$(dirname "$target")
done

if [ "$(id -u)" -eq 0 ] || [ -w "$target" ]; then
    cmake --install build
else
    sudo cmake --install build
fi
