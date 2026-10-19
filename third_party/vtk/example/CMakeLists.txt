# Copyright (c) 2026 The Mogu Authors.
# All rights reserved.

# Headless VTK sample. Qt, Views, and Rendering are not linked.
cmake_minimum_required(VERSION 3.16)
project(vtk_image_grid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(VTK REQUIRED COMPONENTS
  CommonCore
  CommonDataModel
  IOXML
)

add_executable(vtk_image_grid image_grid.cc)
target_link_libraries(vtk_image_grid PRIVATE ${VTK_LIBRARIES})
vtk_module_autoinit(
  TARGETS vtk_image_grid
  MODULES ${VTK_LIBRARIES}
)
