/**
 * @file    ExifTagList.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main ExifTagList class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/ExifTagList.h"

namespace SpatiaLite
{

    ExifTagList::ExifTagList(const unsigned char * blob, int size) :
        Buffer<ExifTagListType>(gaiaGetExifTags(blob, size), gaiaExifTagsFree)
    {
    }

}
