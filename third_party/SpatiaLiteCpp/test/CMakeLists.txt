
ENABLE_TESTING()

FIND_PACKAGE(Threads REQUIRED)

# ======================================================================
# Enable ExternalProject CMake module
# ----------------------------------------------------------------------
INCLUDE(ExternalProject)

# ======================================================================
# Download and install GoogleTest
# ----------------------------------------------------------------------
ExternalProject_Add(
    gtest
    URL https://github.com/google/googletest/archive/release-1.8.0.zip
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
    # Disable install step
    INSTALL_COMMAND ""
)

# ======================================================================
# Create a libgtest target to be used as a dependency by test programs
# ----------------------------------------------------------------------
ADD_LIBRARY(libgtest IMPORTED STATIC GLOBAL)
ADD_DEPENDENCIES(libgtest gtest)

# ======================================================================
# Set gtest properties
# ----------------------------------------------------------------------

ExternalProject_Get_Property(gtest source_dir binary_dir)
IF(WIN32)
    SET_TARGET_PROPERTIES(libgtest PROPERTIES
        "IMPORTED_LOCATION_DEBUG" "${binary_dir}/googlemock/gtest/Debug/gtest.lib"
        "IMPORTED_LOCATION_RELEASE" "${binary_dir}/googlemock/gtest/Release/gtest.lib"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
ELSE()
    SET_TARGET_PROPERTIES(libgtest PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/libgtest.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
ENDIF()
INCLUDE_DIRECTORIES("${source_dir}/googletest/include")

# ======================================================================
# Create a libgmock target to be used as a dependency by test programs
# ----------------------------------------------------------------------
ADD_LIBRARY(libgmock IMPORTED STATIC GLOBAL)

# ======================================================================
# Set gmock properties
# ----------------------------------------------------------------------

IF(WIN32)
    SET_TARGET_PROPERTIES(libgmock PROPERTIES
        "IMPORTED_LOCATION_DEBUG" "${binary_dir}/googlemock/Debug/gmock.lib"
        "IMPORTED_LOCATION_RELEASE" "${binary_dir}/googlemock/Release/gmock.lib"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
ELSE()
    SET_TARGET_PROPERTIES(libgmock PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/libgmock.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
ENDIF()
INCLUDE_DIRECTORIES("${source_dir}/include")

# ======================================================================
# Add test directory
# ----------------------------------------------------------------------
ADD_SUBDIRECTORY(SpatiaLiteCpp)
