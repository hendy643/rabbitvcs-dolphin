# Packaging

Recipes for building RabbitVCS Dolphin as a distribution package.

| Target | Recipe | Output |
|---|---|---|
| Ubuntu 24.04 | `debian/` | `.deb` |
| Ubuntu 26.04 | `debian/` | `.deb` |
| Fedora | `fedora/rabbitvcs-dolphin.spec` | `.rpm` |
| Arch Linux | `arch/PKGBUILD` | `.pkg.tar.zst` |

Build one in a throw-away Docker container (needs Docker and network access):

    packaging/build.sh ubuntu-24.04    # or ubuntu-26.04, fedora, arch

Packages are written to `packaging/dist/<target>/`. Each build also runs the
unit tests.

## Notes

- The plugin needs Qt 6, KDE Frameworks 6 and the Dolphin 6 plugin headers
  (`libdolphinvcs-dev`). **Ubuntu 24.04 cannot build it**: it ships only
  KDE Frameworks 5 and Dolphin 23.08, and `libkf6kio-dev` is not available
  there even with the Kubuntu backports PPA. The `debian/` recipe is the same
  one that builds on 26.04; it would need a KF6 source for 24.04 or a KF5
  port of the plugin.
- `arch/PKGBUILD` is meant for the AUR: it downloads the `v<pkgver>` tag from
  `url`, depends on `rabbitvcs-cli`, and needs a real checksum (`updpkgsums`) once the tag exists. `build.sh arch` also writes
  `.SRCINFO` next to the package.
- The version (`0.1`) is set in `debian/changelog`, the spec, the `PKGBUILD`
  and `build.sh`. Change all four on a release.
- Runtime: the `rabbitvcs` command is only recommended on Debian/Fedora, because
  package names for it differ; the Arch package depends on `rabbitvcs-cli`.
