/**
 * @file    ExifTagList.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main ExifTagList class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

#include "SpatiaLiteCpp/Buffer.hpp"

extern "C"
{
#include "spatialite/gaiaexif.h"
}

namespace SpatiaLite
{

    /**
     * EXIF Tag List buffer data type
     */
    typedef gaiaExifTagListPtr ExifTagListType;

    /**
     * @brief RAII management of a gaiaExifTagListPtr.
     */
    class SPATIALITECPP_ABI ExifTagList : public Buffer<ExifTagListType>
    {

    public:

        /**
         * @brief Creates a list of EXIF tags by parsing a BLOB of the
         *        JPEG-EXIF type.
         * @param[in] blob BLOB to be parsed
         * @param[in] size BLOB size (in bytes)
         */
        ExifTagList(const unsigned char * blob, int size);

    };

}
