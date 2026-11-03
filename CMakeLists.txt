# SPDX-FileCopyrightText: 2026 RabbitVCS contributors
# SPDX-License-Identifier: GPL-2.0-or-later

cmake_minimum_required(VERSION 3.20)
project(rabbitvcs-dolphin-vcs VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(ECM 6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
include(KDEInstallDirs)
include(KDECMakeSettings)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(KF6CoreAddons REQUIRED)
find_package(KF6KIO REQUIRED)
find_package(DolphinVcs REQUIRED)

# Some distributions install the header in <prefix>/include/Dolphin while the
# exported DolphinVcs target only adds <prefix>/include.
find_path(DOLPHINVCS_INCLUDE_DIR NAMES KVersionControlPlugin PATH_SUFFIXES Dolphin)

function(add_rabbitvcs_plugin target source)
    kcoreaddons_add_plugin(${target} INSTALL_NAMESPACE "dolphin/vcs")
    target_sources(${target} PRIVATE ${source} rabbitvcsplugin.cpp)
    target_include_directories(${target} PRIVATE ${DOLPHINVCS_INCLUDE_DIR})
    target_link_libraries(${target} PRIVATE Qt6::Core Qt6::Widgets KF6::KIOCore KF6::KIOGui DolphinVcs)
endfunction()

add_rabbitvcs_plugin(rabbitvcsgitplugin gitplugin.cpp)
add_rabbitvcs_plugin(rabbitvcssvnplugin svnplugin.cpp)
