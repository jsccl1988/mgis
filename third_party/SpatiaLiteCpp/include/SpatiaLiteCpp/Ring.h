/**
 * @file    Ring.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main Ring class.
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
     * Ring buffer data type
     */
    typedef gaiaRingPtr RingType;

    /**
     * @brief RAII management of a gaiaRingPtr.
     */
    class SPATIALITECPP_ABI Ring : public Buffer<RingType>
    {

    public:

        /**
         * @brief Takes ownership of an existing ring pointer
         * @param[in] ring Existing ring pointer
         */
        explicit Ring(RingType ring);

    };

}
