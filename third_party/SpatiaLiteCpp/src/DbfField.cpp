/**
 * @file    DbfField.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main DbfField class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/DbfField.h"

namespace SpatiaLite
{

    DbfField::DbfField(DbfFieldType field) :
        Buffer<DbfFieldType>(field, gaiaFreeDbfField)
    {
    }

}
