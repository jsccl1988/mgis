/**
 * @file    Blob.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main Blob class.
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

namespace SpatiaLite
{

    /**
     * Blob buffer data type
     */
    typedef unsigned char * BlobType;

    /**
     * @brief RAII management of a BLOB array.
     */
    class SPATIALITECPP_ABI Blob : public BufferSized<BlobType>
    {

    public:

        /**
         * @brief Takes ownership of an existing BLOB pointer
         * @param[in] blob Source blob
         * @param[in] size Size of blob
         */
        Blob(BlobType blob, int size);

        /**
         * Cleans up Blob buffer array memory
         * @param[in] buffer Buffer.
         */
        static void clean(BlobType buffer);

        /**
         * @brief Creates a Compressed BLOB-Geometry.
         * @param[in] geometry Input geometry collection
         * @returns New pointer to compressed blob
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer. Should be owned by a BlobPtr.
         */
        static Blob * toCompressedBlobWkb(gaiaGeomCollPtr geometry);

        /**
         * @brief Encodes a Geometry object into FGF notation.
         * @param[in] geometry Input geometry collection
         * @param[in] dimensions Coordinate dimensions type. One of:
         *            GAIA_XY, GAIA_XY_Z, GAIA_XY_M, GAIA_XY_ZM
         * @returns New pointer to FGF blob
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer. Should be owned by a BlobPtr.
         */
        static Blob * toFgf(gaiaGeomCollPtr geometry, int dimensions);

        /**
         * @brief Encodes a Geometry object into (hex) WKB notation.
         * @param[in] geometry Input geometry collection
         * @returns New pointer to Hex WKB blob
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer. Should be owned by a BlobPtr.
         */
        static Blob * toHexWkb(gaiaGeomCollPtr geometry);

        /**
         * @brief Creates a BLOB-Geometry corresponding to a Geometry object.
         * @param[in] geometry Input geometry collection
         * @returns New pointer to SpatiaLite blob
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer. Should be owned by a BlobPtr.
         */
        static Blob * toSpatiaLiteBlobWkb(gaiaGeomCollPtr geometry);

        /**
         * @brief Encodes a Geometry object into WKB notation.
         * @param[in] geometry Input geometry collection
         * @returns New pointer to WKB blob
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer. Should be owned by a BlobPtr.
         */
        static Blob * toWkb(gaiaGeomCollPtr geometry);

    };

}
