/**
 * @file    DbfField.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main DbfField class.
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
     * DBF Field buffer data type
     */
    typedef gaiaDbfFieldPtr DbfFieldType;

    /**
     * @brief RAII management of a gaiaDbfFieldPtr.
     */
    class SPATIALITECPP_ABI DbfField : public Buffer<DbfFieldType>
    {

    public:

        /**
         * @brief Creates a DBF field.
         */
        explicit DbfField(DbfFieldType field);

    };

}
