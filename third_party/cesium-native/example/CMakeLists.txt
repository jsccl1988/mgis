# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.

# Standalone cesium-native sample. The product globe does not link this tree.
cmake_minimum_required(VERSION 3.16)
project(cesium_ellipsoid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(cesium-native REQUIRED)

add_executable(cesium_ellipsoid ellipsoid.cc)
target_link_libraries(cesium_ellipsoid PRIVATE CesiumGeospatial)
