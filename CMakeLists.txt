# SPDX-FileCopyrightText: 2026 RabbitVCS contributors
# SPDX-License-Identifier: GPL-2.0-or-later

cmake_minimum_required(VERSION 3.20)
project(rabbitvcs-dolphin-vcs VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(ECM 6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(CTest) # provides the BUILD_TESTING option

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(KF6CoreAddons REQUIRED)
find_package(KF6KIO REQUIRED)
find_package(DolphinVcs REQUIRED)

# Some distributions install the header in <prefix>/include/Dolphin while the
# exported DolphinVcs target only adds <prefix>/include.
find_path(DOLPHINVCS_INCLUDE_DIR NAMES KVersionControlPlugin PATH_SUFFIXES Dolphin)

add_subdirectory(src)

if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

# Packaging: `cpack -G DEB` / `cpack -G RPM` (see the package-* workflow presets).
# The install layout follows the build host, so build each package on its target family.
set(CPACK_PACKAGE_NAME rabbitvcs-dolphin)
set(CPACK_PACKAGE_VENDOR "RabbitVCS contributors")
set(CPACK_PACKAGE_CONTACT "hendy643 <phenderson643@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "RabbitVCS integration for the Dolphin file manager")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/hendy643/rabbitvcs-dolphin")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")

set(CPACK_DEBIAN_PACKAGE_SECTION kde)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS dolphin)
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS rabbitvcs-cli)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

set(CPACK_RPM_PACKAGE_LICENSE "GPL-2.0-or-later")
set(CPACK_RPM_PACKAGE_REQUIRES dolphin)
set(CPACK_RPM_PACKAGE_SUGGESTS rabbitvcs)
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

include(CPack)
