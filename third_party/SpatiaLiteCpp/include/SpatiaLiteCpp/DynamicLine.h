/**
 * @file    DynamicLine.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main DynamicLine class.
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
     * Dynamic Line buffer data type
     */
    typedef gaiaDynamicLinePtr DynamicLineType;

    /**
     * @brief RAII management of a gaiaDynamicLinePtr.
     */
    class SPATIALITECPP_ABI DynamicLine : public Buffer<DynamicLineType>
    {

    public:

        /**
         * @brief Takes ownership of an existing dynamic line pointer
         * @param[in] line Existing dynamic line pointer
         */
        explicit DynamicLine(DynamicLineType line);

    };

}
