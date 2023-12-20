/**
 * @file    Polygon.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main Polygon class.
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

    /**
     * Polygon buffer data type
     */
    typedef gaiaPolygonPtr PolygonType;

    /**
     * @brief RAII management of a gaiaPolygonPtr.
     */
    class SPATIALITECPP_ABI Polygon : public Buffer<PolygonType>
    {

    public:

        /**
         * @brief Takes ownership of an existing polygon pointer
         * @param[in] polygon Existing polygon pointer
         */
        explicit Polygon(gaiaPolygonPtr polygon);

    };

}
