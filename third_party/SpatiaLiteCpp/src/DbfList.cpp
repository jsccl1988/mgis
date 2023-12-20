/**
 * @file    DbfList.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main DbfList class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/DbfList.h"

namespace SpatiaLite
{

    DbfList::DbfList(DbfListType list) :
        Buffer<DbfListType>(list, gaiaFreeDbfList)
    {
    }

}
