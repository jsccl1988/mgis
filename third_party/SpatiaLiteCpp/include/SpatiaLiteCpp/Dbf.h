/**
 * @file    Dbf.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main Dbf class.
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
     * DBF buffer data type
     */
    typedef gaiaDbfPtr DbfType;

    /**
     * @brief RAII management of a gaiaDbfPtr.
     */
    class SPATIALITECPP_ABI Dbf : public Buffer<DbfType>
    {

    public:

        /**
         * @brief Creates a DBF file.
         */
        explicit Dbf(DbfType dbf);

    };

}
