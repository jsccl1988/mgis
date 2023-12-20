
# ==================================================
# Set include directories
# --------------------------------------------------

SET(spatialitecpptest_inc
    ${sharedptr_dir_inc}
    ${spatialite_dir_inc}
    ${spatialitecpp_dir_inc}
    ${sqlitecpp_dir_inc})
INCLUDE_DIRECTORIES(${spatialitecpptest_inc})

# ==================================================
# Add test source files
# --------------------------------------------------

FILE(GLOB spatialitecpptest_src *.cpp)

# ==================================================
# Add test executable
# --------------------------------------------------

# Link as static library if dynamic option not set
IF(WIN32 AND ${SPATIALITECPP_BUILD_DYNAMIC})
    ADD_DEFINITIONS(-DSPATIALITECPP_IMPORT)
ENDIF()

ADD_EXECUTABLE(SpatiaLiteCppTest ${spatialitecpptest_src})
TARGET_COMPILE_DEFINITIONS(SpatiaLiteCppTest PRIVATE
    SPATIALITECPP_TEST_EX_DIR="${CMAKE_BINARY_DIR}/examples")

TARGET_LINK_LIBRARIES(SpatiaLiteCppTest
    libgtest
    libgmock
    ${spatialite_lib}
    SpatiaLiteCpp
    ${sqlite3_lib}
    SQLiteCpp
)

ADD_TEST(NAME SpatiaLiteCppTest
         COMMAND SpatiaLiteCppTest)
