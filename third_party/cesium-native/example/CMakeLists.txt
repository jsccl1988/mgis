# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.

# Standalone cesium-native sample. The product globe does not link this tree.
cmake_minimum_required(VERSION 3.16)

# Installed cesium-native prefix (headers + Cesium*.lib).
get_filename_component(_mview_tp "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
list(APPEND CMAKE_PREFIX_PATH "${_mview_tp}/.install")

# ezvcpkg deps used when cesium-native was configured (modp_b64, zlib-ng, …).
# Override with: -DCMAKE_TOOLCHAIN_FILE=<ezvcpkg>/scripts/buildsystems/vcpkg.cmake
# -DVCPKG_TARGET_TRIPLET=x64-windows-static-md
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR CMAKE_TOOLCHAIN_FILE STREQUAL "")
  set(_ez_base "$ENV{EZVCPKG_BASEDIR}")
  if(_ez_base STREQUAL "")
    set(_ez_base "C:/cn-vcpkg")
  endif()
  file(GLOB _ez_toolchains "${_ez_base}/*/scripts/buildsystems/vcpkg.cmake")
  if(_ez_toolchains)
    list(GET _ez_toolchains 0 CMAKE_TOOLCHAIN_FILE)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "" FORCE)
    message(STATUS "cesium example: using ${CMAKE_TOOLCHAIN_FILE}")
  endif()
endif()

project(cesium_ellipsoid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(cesium-native REQUIRED)

add_executable(cesium_ellipsoid ellipsoid.cc)
target_link_libraries(cesium_ellipsoid PRIVATE CesiumGeospatial)
