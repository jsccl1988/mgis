/**
 * @file    WfsSchema.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main WfsSchema class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

#include "SpatiaLiteCpp/Buffer.hpp"

extern "C"
{
#include "sqlite3.h"
#include "spatialite.h"
#include "spatialite/gg_wfs.h"
}

#include <string>

namespace SpatiaLite
{

    /**
     * WFS schema buffer data type
     */
    typedef gaiaWFSschemaPtr WfsSchemaType;

    /**
     * @brief RAII management of a gaiaWFSschemaPtr.
     */
    class SPATIALITECPP_ABI WfsSchema : public Buffer<WfsSchemaType>
    {

    public:

        /**
         * @brief Create WFS schema
         * @param[in] path  Pointer to some WFS-DescribeFeatureType XML
         *                  Document (could be a pathname or an URL).
         * @param[in] layer Layer name.
         * @returns New WfsSchema pointer
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer.
         *          Should be owned by a WfsSchemaPtr.
         */
        static WfsSchema * create(std::string const & path,
                                  std::string const & layer);

    private:

        /**
         * @brief Takes ownership of an existing WFS schema pointer
         * @param[in] schema Pointer to WFS schema.
         */
        explicit WfsSchema(WfsSchemaType schema);

    };

}
