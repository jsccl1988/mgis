# 
# This CMake file is a local alternate to the one
# that comes with SQLiteC++. This build maintains
# consistent run-time libraries.
# 

# ==================================================
# Set external directories
# --------------------------------------------------

SET(sqlitecpp_dir "${spatialitecpp_dir}/ext/SQLiteCpp")

# ==================================================
# Set header files
# --------------------------------------------------

SET(sqlitecpp_hdr
    "${sqlitecpp_dir}/include/SQLiteCpp/Assertion.h"
    "${sqlitecpp_dir}/include/SQLiteCpp/Backup.h"
    "${sqlitecpp_dir}/include/SQLiteCpp/Column.h"
    "${sqlitecpp_dir}/include/SQLiteCpp/Database.h"
    "${sqlitecpp_dir}/include/SQLiteCpp/Exception.h"
    "${sqlitecpp_dir}/include/SQLiteCpp/SQLiteCpp.h"
    "${sqlitecpp_dir}/include/SQLiteCpp/Statement.h"
    "${sqlitecpp_dir}/include/SQLiteCpp/Transaction.h"
    "${sqlitecpp_dir}/include/SQLiteCpp/VariadicBind.h")

# ==================================================
# Set source files
# --------------------------------------------------

SET(sqlitecpp_src
    "${sqlitecpp_dir}/src/Backup.cpp"
    "${sqlitecpp_dir}/src/Column.cpp"
    "${sqlitecpp_dir}/src/Database.cpp"
    "${sqlitecpp_dir}/src/Statement.cpp"
    "${sqlitecpp_dir}/src/Transaction.cpp")

# ==================================================
# Set include directories
# --------------------------------------------------

SET(sqlitecpp_inc
    "${sqlitecpp_dir}/include"
    "${sqlitecpp_dir}/sqlite3")
INCLUDE_DIRECTORIES(${sqlitecpp_inc})

# ==================================================
# Add executable and link dependencies
# --------------------------------------------------

IF(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF()
ADD_LIBRARY(SQLiteCpp STATIC ${sqlitecpp_src} ${sqlitecpp_hdr})
SET_TARGET_PROPERTIES(SQLiteCpp PROPERTIES LINKER_LANGUAGE CXX)
IF(UNIX)
    TARGET_LINK_LIBRARIES(SQLiteCpp ${sqlite3_lib})
ENDIF()