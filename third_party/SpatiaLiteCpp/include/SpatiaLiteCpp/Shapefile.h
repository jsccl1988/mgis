/**
 * @file    Shapefile.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main Shapefile class.
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
     * Shapefile buffer data type
     */
    typedef gaiaShapefilePtr ShapefileType;

    /**
     * @brief RAII management of a gaiaShapefilePtr.
     */
    class SPATIALITECPP_ABI Shapefile : public Buffer<ShapefileType>
    {

    public:

        /**
         * @brief Creates a Shapefile.
         */
        explicit Shapefile(ShapefileType shapefile);

    };

}
