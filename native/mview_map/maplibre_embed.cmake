# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.

# Built by MapLibre's CMake so this DLL shares that library's CRT and
# exception flags. GN links mview_map.dll and LoadLibrary's this one.

get_filename_component(MVIEW_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

add_library(mview_maplibre SHARED
    "${CMAKE_CURRENT_LIST_DIR}/maplibre_present.cc")

get_target_property(MBGL_LINK_LIBS mbgl-core LINK_LIBRARIES)
if(NOT MBGL_LINK_LIBS)
    set(MBGL_LINK_LIBS "")
endif()

target_link_libraries(mview_maplibre PRIVATE mbgl-core ${MBGL_LINK_LIBS})

target_include_directories(mview_maplibre PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/platform/default/include"
    "${PROJECT_SOURCE_DIR}/vendor/maplibre-native-base/include"
    "${PROJECT_SOURCE_DIR}/vendor/boost/include"
    "${PROJECT_SOURCE_DIR}/vendor/expected-lite/include"
    "${PROJECT_SOURCE_DIR}/vendor/filesystem/include")

target_compile_features(mview_maplibre PRIVATE cxx_std_20)
target_compile_options(mview_maplibre PRIVATE /EHsc /utf-8)
target_compile_definitions(mview_maplibre PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN)

set_target_properties(mview_maplibre PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${MVIEW_ROOT}/out/Debug"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${MVIEW_ROOT}/out/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${MVIEW_ROOT}/out/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${MVIEW_ROOT}/out/Debug")

target_link_libraries(mview_maplibre PRIVATE opengl32 gdi32 user32)
