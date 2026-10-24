# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.

# Cesium for FlyCube. Not part of the views GN graph.
cmake_minimum_required(VERSION 3.20)

get_filename_component(_mview_tp "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
list(APPEND CMAKE_PREFIX_PATH "${_mview_tp}/.install")

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR CMAKE_TOOLCHAIN_FILE STREQUAL "")
  set(_ez_base "$ENV{EZVCPKG_BASEDIR}")
  if(_ez_base STREQUAL "")
    set(_ez_base "C:/cn-vcpkg")
  endif()
  file(GLOB _ez_toolchains "${_ez_base}/*/scripts/buildsystems/vcpkg.cmake")
  if(_ez_toolchains)
    list(GET _ez_toolchains 0 CMAKE_TOOLCHAIN_FILE)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "" FORCE)
    message(STATUS "cesium-flycube: using ${CMAKE_TOOLCHAIN_FILE}")
  endif()
endif()

project(cesium_flycube LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
  add_compile_options(/utf-8)
endif()

find_package(cesium-native REQUIRED)

add_library(cesium_flycube STATIC
  cesium_flycube/geo/georeference.cc
  cesium_flycube/geo/globe_anchor.cc
  cesium_flycube/geo/origin_shift.cc
  cesium_flycube/view/origin_policy.cc
  cesium_flycube/view/view_state_builder.cc
  cesium_flycube/view/session.cc
  cesium_flycube/tiles/tile_mesh.cc
  cesium_flycube/tiles/feature_table.cc
  cesium_flycube/tiles/raster_image.cc
  cesium_flycube/tiles/raster_overlay.cc
  cesium_flycube/tiles/prepare_resources.cc
  cesium_flycube/tiles/frame.cc
  cesium_flycube/tiles/tileset_services.cc
  cesium_flycube/tiles/tileset_settings.cc
  cesium_flycube/tiles/tileset_host.cc
)
target_include_directories(cesium_flycube PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
target_link_libraries(cesium_flycube
  PUBLIC
    Cesium3DTilesSelection
    CesiumCurl
    CesiumGeospatial
    CesiumGltf
    CesiumUtility
    CesiumAsync
    CesiumRasterOverlays
    CesiumVectorOverlays
    CesiumImage
)

set(_flycube_root "${_mview_tp}/.src/flycube")
set(_flycube_lib "${_flycube_root}/out/lib")

add_library(cesium_flycube_gpu STATIC
  cesium_flycube/gpu/mesh_pack.cc
  cesium_flycube/gpu/flycube_upload.cc
  cesium_flycube/gpu/frame_present.cc
  cesium_flycube/gpu/draped_terrain.cc
  cesium_flycube/gpu/scene_present.cc
)
target_include_directories(cesium_flycube_gpu
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
  PRIVATE
    "${_flycube_root}/src/FlyCube"
    "${_flycube_root}/3rdparty/glm"
    "${_flycube_root}/3rdparty/gli"
    "${_flycube_root}/3rdparty/Vulkan-Headers/include"
    "${_flycube_root}/3rdparty/DirectX-Headers/include"
    "${_flycube_root}/3rdparty/DirectX-Headers/include/directx"
    "${_flycube_root}/3rdparty/nowide/include"
)
target_compile_definitions(cesium_flycube_gpu PRIVATE
  DIRECTX_SUPPORT
  NOMINMAX
  CESIUM_FLYCUBE_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/cesium_flycube/gpu/shaders"
)
target_link_directories(cesium_flycube_gpu PUBLIC "${_flycube_lib}")
target_link_libraries(cesium_flycube_gpu
  PUBLIC
    cesium_flycube
    FlyCube
    DirectX-Headers
    nowide
    spirv-cross-hlsl
    spirv-cross-msl
    spirv-cross-glsl
    spirv-cross-core
    d3d12
    dxgi
    dxguid
    windowscodecs
    ole32
)

function(cesium_flycube_example name src)
  add_executable(${name} ${src})
  target_link_libraries(${name} PRIVATE cesium_flycube cesium_flycube_gpu)
  target_compile_definitions(${name} PRIVATE
    CESIUM_FLYCUBE_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example/data"
    CESIUM_FLYCUBE_CAPTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example/capture"
  )
endfunction()

cesium_flycube_example(cesium_flycube_dual_view example/dual_view.cc)
cesium_flycube_example(cesium_flycube_ellipsoid_city example/ellipsoid_city.cc)
cesium_flycube_example(cesium_flycube_align_native example/align_native.cc)
cesium_flycube_example(cesium_flycube_globe_city example/globe_city.cc)

# Product WorldPass sources live in //vista/pass/world (GN copies the DLL).
set(_vista_world_src "${CMAKE_CURRENT_LIST_DIR}/../../vista/pass/world")
add_library(vista_world SHARED
  "${_vista_world_src}/world_pass.cc"
  "${_vista_world_src}/world_pass.h"
)
target_include_directories(vista_world
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/../.."
  PRIVATE
    "${_flycube_root}/src/FlyCube"
    "${_flycube_root}/3rdparty/glm"
    "${_flycube_root}/3rdparty/gli"
    "${_flycube_root}/3rdparty/Vulkan-Headers/include"
    "${_flycube_root}/3rdparty/DirectX-Headers/include"
    "${_flycube_root}/3rdparty/DirectX-Headers/include/directx"
    "${_flycube_root}/3rdparty/nowide/include"
)
target_compile_definitions(vista_world PRIVATE
  VISTA_WORLD_IMPLEMENTATION
  DIRECTX_SUPPORT
  NOMINMAX
  CESIUM_FLYCUBE_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/cesium_flycube/gpu/shaders"
  CESIUM_FLYCUBE_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example/data"
)
target_link_directories(vista_world PRIVATE "${_flycube_lib}")
target_link_libraries(vista_world PRIVATE
  cesium_flycube_gpu
  FlyCube
  DirectX-Headers
  nowide
  spirv-cross-hlsl
  spirv-cross-msl
  spirv-cross-glsl
  spirv-cross-core
  d3d12
  dxgi
  dxguid
  user32
  gdi32
)
set_target_properties(vista_world PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
)
cesium_flycube_example(cesium_flycube_google_earth example/google_earth.cc)
cesium_flycube_example(cesium_flycube_browse example/browse_city.cc)
cesium_flycube_example(cesium_flycube_geology example/geology.cc)
cesium_flycube_example(cesium_flycube_groundwater example/groundwater.cc)
cesium_flycube_example(cesium_flycube_river_flood example/river_flood.cc)
cesium_flycube_example(cesium_flycube_wind_field example/wind_field.cc)
