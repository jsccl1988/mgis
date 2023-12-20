/**
 * @file    DbfList.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main DbfList class.
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
     * DBF List buffer data type
     */
    typedef gaiaDbfListPtr DbfListType;

    /**
     * @brief RAII management of a gaiaDbfListPtr.
     */
    class SPATIALITECPP_ABI DbfList : public Buffer<DbfListType>
    {

    public:

        /**
         * @brief Creates a DBF list.
         */
        explicit DbfList(DbfListType list);

    };

}
