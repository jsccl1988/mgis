/**
 * @file    DynamicLine.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main DynamicLine class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/DynamicLine.h"

namespace SpatiaLite
{

    DynamicLine::DynamicLine(DynamicLineType line) :
        Buffer<DynamicLineType>(line, gaiaFreeDynamicLine)
    {
    }

}
