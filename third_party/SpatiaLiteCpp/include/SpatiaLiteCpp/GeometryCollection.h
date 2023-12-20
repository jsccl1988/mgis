/**
 * @file    GeometryCollection.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main GeometryCollection class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

#include "SpatiaLiteCpp/Buffer.hpp"

extern "C"
{
#include "sqlite3.h"
#include "spatialite.h"
}

#include <string>

// Forward declarations
namespace SQLite
{
    class Column;
}

namespace SpatiaLite
{

    // Forward declarations
    class Blob;
    class SpatialDatabase;

    /**
     * Geometry collection buffer data type
     */
    typedef gaiaGeomCollPtr GeometryCollectionType;

    /**
     * @brief RAII management of a gaiaGeomCollPtr.
     */
    class SPATIALITECPP_ABI GeometryCollection :
        public Buffer<GeometryCollectionType>
    {

    public:

        /**
         * @brief Takes ownership of an existing geometry pointer
         * @param[in] geometry Existing Geometry Collection pointer
         */
        explicit GeometryCollection(GeometryCollectionType geometry);

        /**
         * @brief Extract the geometry from a BLOB and take ownership.
         * @param[in] blob Source blob for geometry collection
         */
        explicit GeometryCollection(SQLite::Column const & blob);

        /**
         * @brief Extract the geometry from a BLOB and take ownership.
         * @param[in] blob Source blob for geometry collection
         */
        explicit GeometryCollection(SpatiaLite::Blob const & blob);

    };

}
