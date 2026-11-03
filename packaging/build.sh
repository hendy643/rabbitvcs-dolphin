#!/bin/sh
# Build a distribution package inside a throw-away Docker container.
#   packaging/build.sh ubuntu-24.04 | ubuntu-26.04 | fedora | arch
# Packages end up in packaging/dist/<target>/.
set -eu

target="${1:?usage: $0 ubuntu-24.04|ubuntu-26.04|fedora|arch}"
here=$(cd "$(dirname "$0")" && pwd)
root=$(dirname "$here")
name=rabbitvcs-dolphin
version=0.1
out="$here/dist/$target"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
mkdir -p "$out"

# Source tarball of the working tree (tracked and untracked, minus ignored files).
(cd "$root" && git ls-files -co --exclude-standard -z |
    tar --null -T - --transform "s,^,$name-$version/," -czf "$work/$name-$version.tar.gz")

run() { # run <image> <script>
    docker run --rm -e "OWNER=$(id -u):$(id -g)" -v "$work:/work" -v "$here:/packaging:ro" -v "$out:/out" "$1" \
        sh -eu -c "trap 'chown -R \$OWNER /work /out' EXIT; $2"
}

case "$target" in
ubuntu-24.04 | ubuntu-26.04)
    image="ubuntu:${target#ubuntu-}"
    # 24.04 ships Qt 6.4 and KDE Frameworks 5 / Dolphin 23.08 only (no libkf6kio-dev,
    # even with the Kubuntu backports PPA), so the build is expected to fail there.
    run "$image" "
        export DEBIAN_FRONTEND=noninteractive
        apt-get update && apt-get install -y build-essential debhelper devscripts equivs
        cd /work && tar xf $name-$version.tar.gz && cd $name-$version
        cp -r /packaging/debian debian
        mk-build-deps -ir -t 'apt-get -y --no-install-recommends' debian/control
        dpkg-buildpackage -us -uc -b
        cp /work/*.deb /out/"
    ;;
fedora)
    run fedora:latest "
        dnf install -y rpm-build rpmdevtools dnf-plugins-core
        mkdir -p /root/rpmbuild/SOURCES
        cp /work/$name-$version.tar.gz /root/rpmbuild/SOURCES/
        dnf builddep -y /packaging/fedora/$name.spec
        rpmbuild -ba /packaging/fedora/$name.spec
        cp /root/rpmbuild/RPMS/*/*.rpm /out/"
    ;;
arch)
    run archlinux:latest "
        pacman -Syu --noconfirm base-devel cmake extra-cmake-modules qt6-base kio dolphin git
        useradd -m builder
        mkdir /build && cp /work/$name-$version.tar.gz /packaging/arch/PKGBUILD /build/ && chown -R builder /build
        cd /build && su builder -c 'makepkg -f -d --noconfirm && makepkg --printsrcinfo > .SRCINFO'
        cp /build/*.pkg.tar.* /build/.SRCINFO /out/"
    ;;
*)
    echo "unknown target: $target" >&2
    exit 1
    ;;
esac
echo "Packages written to $out"
