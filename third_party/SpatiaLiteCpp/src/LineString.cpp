/**
 * @file    LineString.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main LineString class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/LineString.h"

#include "SpatiaLiteCpp/Blob.h"

#include <stdexcept>

namespace SpatiaLite
{

    LineString::LineString(LineStringType line) :
        Buffer<LineStringType>(line, gaiaFreeLinestring)
    {
    }

    SpatiaLite::Blob * LineString::makeLine(gaiaGeomCollPtr geometry1,
                                            gaiaGeomCollPtr geometry2)
    {
        int size = 0;
        unsigned char * blob = 0;
        if (!geometry1) throw std::runtime_error("Invalid geometry #1!");
        if (!geometry2) throw std::runtime_error("Invalid geometry #2!");
        gaiaMakeLine(geometry1, geometry2, &blob, &size);
        if (!blob) throw std::runtime_error("Failed to make line!");
        return new SpatiaLite::Blob(blob, size);
    }

}
