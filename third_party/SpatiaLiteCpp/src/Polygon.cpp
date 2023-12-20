/**
 * @file    Polygon.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main Polygon class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/Polygon.h"

namespace SpatiaLite
{

    Polygon::Polygon(PolygonType polygon) :
        Buffer<PolygonType>(polygon, gaiaFreePolygon)
    {
    }

}
