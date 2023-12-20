/**
 * @file    VectorLayersList.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main vector layers list class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/VectorLayersList.h"

extern "C"
{
#include "spatialite.h"
}

namespace SpatiaLite
{

    VectorLayersList::VectorLayersList(VectorLayersListType list) :
        Buffer<VectorLayersListType>(list, gaiaFreeVectorLayersList)
    {
    }

}
