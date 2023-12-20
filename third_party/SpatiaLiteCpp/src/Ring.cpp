/**
 * @file    Ring.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main Ring class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/Ring.h"

namespace SpatiaLite
{

    Ring::Ring(RingType ring) :
        Buffer<RingType>(ring, gaiaFreeRing)
    {
    }

}
