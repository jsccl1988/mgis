/**
 * @file    Blob.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main Blob class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/Blob.h"

#include <cstring>
#include <stdexcept>

namespace SpatiaLite
{

    Blob::Blob(BlobType blob, int size) :
        BufferSized<BlobType>(blob, size, Blob::clean)
    {
    }

    void Blob::clean(BlobType buffer)
    {
        if (buffer)
        {
            gaiaFree(buffer);
        }
    }

    Blob * Blob::toCompressedBlobWkb(gaiaGeomCollPtr geometry)
    {
        int size = 0;
        BlobType blob = 0;
        if (!geometry) throw std::runtime_error("Invalid geometry!");
        gaiaToCompressedBlobWkb(geometry, &blob, &size);
        if (!blob) throw std::runtime_error("Failed to compress blob!");
        return new SpatiaLite::Blob(blob, size);
    }

    Blob * Blob::toFgf(gaiaGeomCollPtr geometry, int dimensions)
    {
        int size = 0;
        BlobType blob = 0;
        if (!geometry) throw std::runtime_error("Invalid geometry!");
        gaiaToFgf(geometry, &blob, &size, dimensions);
        if (!blob) throw std::runtime_error("Failed to convert to FGF!");
        return new SpatiaLite::Blob(blob, size);
    }

    Blob * Blob::toHexWkb(gaiaGeomCollPtr geometry)
    {
        if (!geometry) throw std::runtime_error("Invalid geometry!");
        char * blob = gaiaToHexWkb(geometry);
        if (!blob) throw std::runtime_error("Failed to convert to HEX WKB!");
        int size = (int)std::strlen(blob);
        return new SpatiaLite::Blob((BlobType)blob, size);
    }

    Blob * Blob::toSpatiaLiteBlobWkb(gaiaGeomCollPtr geometry)
    {
        int size = 0;
        BlobType blob = 0;
        if (!geometry) throw std::runtime_error("Invalid geometry!");
        gaiaToSpatiaLiteBlobWkb(geometry, &blob, &size);
        if (!blob) throw std::runtime_error("Failed to convert to Blob!");
        return new SpatiaLite::Blob(blob, size);
    }

    Blob * Blob::toWkb(gaiaGeomCollPtr geometry)
    {
        int size = 0;
        BlobType blob = 0;
        if (!geometry) throw std::runtime_error("Invalid geometry!");
        gaiaToWkb(geometry, &blob, &size);
        if (!blob) throw std::runtime_error("Failed to convert to WKB!");
        return new SpatiaLite::Blob(blob, size);
    }

}
