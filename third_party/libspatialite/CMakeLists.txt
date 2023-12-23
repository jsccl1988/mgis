PROJECT(spatialite)
cmake_minimum_required(VERSION 2.8)

include(GNUInstallDirs)
include(ExternalProject)


install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/../)
install(DIRECTORY lib DESTINATION ${CMAKE_INSTALL_LIBDIR}/../)
install(DIRECTORY bin DESTINATION ${CMAKE_INSTALL_BINDIR}/../)
