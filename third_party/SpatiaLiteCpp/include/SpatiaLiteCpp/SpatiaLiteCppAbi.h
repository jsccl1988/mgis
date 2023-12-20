/**
 * @file    SpatiaLiteCppAbi.h
 * @ingroup SpatiaLiteCpp
 * @brief   Defines library export ABI.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

/**
 * @def SPATIALITECPP_ABI
 * @brief Defines the library ABI for Windows otherwise it is blank.
 * @details The following combinations of ABIs are supported
 *              - SPATIALITECPP_EXPORT is defined then a dynamic library is
 *                being built and the DLL export spec is defined.
 *              - SPATIALITECPP_IMPORT is defined then a dynamic library is
 *                being used and the DLL import spec is defined.
 *              - Neither SPATIALITECPP_IMPORT nor SPATIALITECPP_EXPORT are
 *                defined then a static library is being built or used.
 */
#ifdef _WIN32
    #ifdef SPATIALITECPP_EXPORT
        #define SPATIALITECPP_ABI __declspec(dllexport)
    #else
        #ifdef SPATIALITECPP_IMPORT
            #define SPATIALITECPP_ABI __declspec(dllimport)
        #else
            #define SPATIALITECPP_ABI
        #endif
    #endif
#else
    #define SPATIALITECPP_ABI
#endif
