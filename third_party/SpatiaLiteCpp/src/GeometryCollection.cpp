/**
 * @file    GeometryCollection.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main GeometryCollection class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/GeometryCollection.h"

#include "SpatiaLiteCpp/Auxiliary.h"
#include "SpatiaLiteCpp/Blob.h"
#include "SpatiaLiteCpp/SpatialDatabase.h"

#include "SQLiteCpp/SQLiteCpp.h"

namespace SpatiaLite
{

    GeometryCollection::GeometryCollection(GeometryCollectionType geometry) :
        Buffer<GeometryCollectionType>(geometry, gaiaFreeGeomColl)
    {
    }

    GeometryCollection::GeometryCollection(SQLite::Column const & blob) :
        Buffer<GeometryCollectionType>(gaiaFromSpatiaLiteBlobWkb(
                                           (const unsigned char*)blob.getBlob(),
                                           blob.getBytes()),
                                       gaiaFreeGeomColl)
    {
    }

    GeometryCollection::GeometryCollection(SpatiaLite::Blob const & blob) :
        Buffer<GeometryCollectionType>(gaiaFromSpatiaLiteBlobWkb(
                                           blob.get(),
                                           blob.getSize()),
                                       gaiaFreeGeomColl)
    {
    }

}
