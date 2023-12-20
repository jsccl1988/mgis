/**
 * @file    Shapefile.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main Shapefile class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/Shapefile.h"

namespace SpatiaLite
{

    Shapefile::Shapefile(ShapefileType shapefile) :
        Buffer<ShapefileType>(shapefile, gaiaFreeShapefile)
    {
    }

}
