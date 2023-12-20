/**
 * @file    LineString.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main LineString class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

#include "SpatiaLiteCpp/Buffer.hpp"

extern "C"
{
#include "sqlite3.h"
#include "spatialite/gaiageo.h"
}

namespace SpatiaLite
{

    // Forward declarations
    class Blob;

    /**
     * Line string buffer data type
     */
    typedef gaiaLinestringPtr LineStringType;

    /**
     * @brief RAII management of a gaiaLinestringPtr.
     */
    class SPATIALITECPP_ABI LineString : public Buffer<LineStringType>
    {

    public:

        /**
         * @brief Takes ownership of an existing line string pointer
         * @param[in] line Existing line pointer
         */
        explicit LineString(LineStringType line);

        /**
         * @brief Create new line string from two points
         * @param[in] geometry1 Point #1
         * @param[in] geometry2 Point #2
         * @returns New pointer of line string geometry blob
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer. Should be owned by a BlobPtr.
         */
        static SpatiaLite::Blob * makeLine(gaiaGeomCollPtr geometry1,
                                           gaiaGeomCollPtr geometry2);

    };

}
