/**
 * @file    Point.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main Point class.
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
     * Point buffer data type
     */
    typedef gaiaPointPtr PointType;

    /**
     * @brief RAII management of a gaiaPointPtr.
     */
    class SPATIALITECPP_ABI Point : public Buffer<PointType>
    {

    public:

        /**
         * @brief Takes ownership of an existing point pointer
         * @param[in] point Existing point pointer
         */
        explicit Point(PointType point);

        /**
         * @brief Create new point from coordinates
         * @param[in] srid Coordinate reference system code
         * @param[in] x Point x-coordinate
         * @param[in] y Point y-coordinate
         * @param[in] z Point z-coordinate (optional)
         * @param[in] m Point measurement (optional)
         * @returns New pointer of point geometry blob. The geometry blob can be
         *          of type Point, PointM, PointZ, or PointZM depending if the
         *          z-coordinate or measurement are provided.
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer. Should be owned by a BlobPtr.
         */
        static SpatiaLite::Blob * makePoint(int srid,
                                            double x,
                                            double y,
                                            double * z = 0,
                                            double * m = 0);

    };

}
