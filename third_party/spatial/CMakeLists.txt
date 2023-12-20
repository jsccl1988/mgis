cmake_minimum_required (VERSION 2.8.4)
project (spatial_examples)

include(GNUInstallDirs)
include(ExternalProject)

include_directories(SYSTEM ./)
add_executable (runtime_container_rank examples/runtime_container_rank.cpp)
add_executable (get_container_size examples/get_container_size.cpp)
add_executable (custom_accessor_less examples/custom_accessor_less.cpp)

install(DIRECTORY spatial DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

