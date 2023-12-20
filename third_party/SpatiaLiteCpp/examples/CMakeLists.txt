
# ==================================================
# Set examples directory and files
# --------------------------------------------------

SET(examples_dir_data "${spatialitecpp_dir}/examples")
SET(examples_src
    "${CMAKE_CURRENT_SOURCE_DIR}/demos.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/examples.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/samples.cpp")

# ==================================================
# Add executable and link dependencies
# --------------------------------------------------

# Link as static library if dynamic option not set
IF(WIN32 AND ${SPATIALITECPP_BUILD_DYNAMIC})
    ADD_DEFINITIONS(-DSPATIALITECPP_IMPORT)
ENDIF()

ADD_EXECUTABLE(examples ${examples_src})
TARGET_LINK_LIBRARIES(examples
    ${spatialite_lib}
    ${sqlite3_lib}
    SQLiteCpp
    SpatiaLiteCpp)

# ==================================================
# Set include directories
# --------------------------------------------------

SET(examples_inc
    ${sharedptr_dir_inc}
    ${spatialitecpp_dir_inc}
    ${sqlitecpp_dir_inc}
    ${spatialite_dir_inc})
INCLUDE_DIRECTORIES(${examples_inc})

# ==================================================
# Add executable to install project
# --------------------------------------------------

SET(install_dir_ex "${install_dir}/examples")

INSTALL(TARGETS examples RUNTIME DESTINATION ${install_dir_ex}/)
INSTALL(DIRECTORY "${examples_dir_data}/spatialite" DESTINATION ${install_dir_ex})
INSTALL(DIRECTORY "${examples_dir_data}/states" DESTINATION ${install_dir_ex})
INSTALL(DIRECTORY "${examples_dir_data}/wfs" DESTINATION ${install_dir_ex})
INSTALL(DIRECTORY "${examples_dir_data}/xml" DESTINATION ${install_dir_ex})
IF(${SPATIALITECPP_BUILD_DYNAMIC})
    INSTALL(DIRECTORY "${install_dir}/lib/"
            DESTINATION ${install_dir_ex}
            FILES_MATCHING PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}")
ENDIF()

# ==================================================
# Copy example files
# --------------------------------------------------

SET(examples_build_dir "${CMAKE_BINARY_DIR}/examples")

FILE(COPY "${examples_dir_data}/spatialite" DESTINATION ${examples_build_dir})
FILE(COPY "${examples_dir_data}/states" DESTINATION ${examples_build_dir})
FILE(COPY "${examples_dir_data}/wfs" DESTINATION ${examples_build_dir})
FILE(COPY "${examples_dir_data}/xml" DESTINATION ${examples_build_dir})
