CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# ==================================================
# Defines CPack properties for SpatiaLiteC++ installer
# --------------------------------------------------

SET(CPACK_PACKAGE_NAME "SpatiaLiteCpp")
SET(CPACK_PACKAGE_VENDOR "dpmcmlxxvi")
SET(CPACK_PACKAGE_CONTACT "dpmcmlxxvi@gmail.com")

# Version needs to be populated by the build process
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")

# This is a hack to add build type to installer name since
# CPACK_SYSTEM_NAME is not available at compile time.
SET(build_suffix ${CMAKE_BUILD_TYPE})
IF(BUILD_TYPE)
    SET(build_suffix ${BUILD_TYPE})
ENDIF()
SET(CPACK_PACKAGE_VERSION_PATCH "${CPACK_PACKAGE_VERSION_PATCH}-${build_suffix}")

SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}.${CPACK_PACKAGE_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "SpatiaLiteC++ is a C++ wrapper for the SpatiaLite library to provide RAII management.")
SET(CPACK_GENERATOR
    "ZIP"
    "TGZ")

INCLUDE(CPack)
