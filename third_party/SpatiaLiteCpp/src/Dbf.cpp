/**
 * @file    Dbf.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main Dbf class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/Dbf.h"

namespace SpatiaLite
{

    Dbf::Dbf(DbfType dbf) :
        Buffer<DbfType>(dbf, gaiaFreeDbf)
    {
    }

}
