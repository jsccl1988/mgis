/**
 * @file    Point.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main Point class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/Point.h"

#include "SpatiaLiteCpp/Blob.h"

#include <stdexcept>

namespace SpatiaLite
{

    Point::Point(PointType point) :
        Buffer<PointType>(point, gaiaFreePoint)
    {
    }

    SpatiaLite::Blob * Point::makePoint(int srid,
                                        double x,
                                        double y,
                                        double * z,
                                        double * m)
    {
        int size = 0;
        unsigned char * blob = 0;
        if (z == 0 && m == 0)
        {
            gaiaMakePoint(x, y, srid, &blob, &size);
        }
        else if (z == 0)
        {
            gaiaMakePointM(x, y, *m, srid, &blob, &size);
        }
        else if (m == 0)
        {
            gaiaMakePointZ(x, y, *z, srid, &blob, &size);
        }
        else
        {
            gaiaMakePointZM(x, y, *z, *m, srid, &blob, &size);
        }
        if (!blob) throw std::runtime_error("Failed to make point!");
        return new SpatiaLite::Blob(blob, size);
    }

}
