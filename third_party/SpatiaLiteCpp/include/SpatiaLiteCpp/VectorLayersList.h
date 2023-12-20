/**
 * @file    VectorLayersList.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main vector layers list class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

#include "SpatiaLiteCpp/Buffer.hpp"

extern "C"
{
#include "sqlite3.h"
#include "spatialite/gaiageo.h"
}

namespace SpatiaLite
{

    /**
     * Vector layers list buffer data type
     */
    typedef gaiaVectorLayersListPtr VectorLayersListType;

    /**
     * @brief RAII management of a gaiaVectorLayersListPtr.
     */
    class SPATIALITECPP_ABI VectorLayersList :
        public Buffer<VectorLayersListType>
    {

    public:

        /**
         * @brief Create new layer list
         * @param[in] list Input Vector Layers List
         */
        explicit VectorLayersList(VectorLayersListType list);

    };

}
