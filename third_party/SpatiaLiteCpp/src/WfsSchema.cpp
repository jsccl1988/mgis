/**
 * @file    WfsSchema.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main WfsSchema class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/WfsSchema.h"

#include "SpatiaLiteCpp/Auxiliary.h"

#include <stdexcept>

namespace SpatiaLite
{

    WfsSchema::WfsSchema(WfsSchemaType schema) :
        Buffer<WfsSchemaType>(schema, destroy_wfs_schema)
    {
    }

    WfsSchema * WfsSchema::create(std::string const & path,
                                  std::string const & layer)
    {
        char * message = 0;
        WfsSchemaType schema = create_wfs_schema(path.c_str(),
                                                 layer.c_str(),
                                                 &message);
        if (!schema)
        {
            std::string error = Auxiliary::toString(
                message, "Failed to create WFS schema!");
            throw std::runtime_error(error);
        }
        return new WfsSchema(schema);
    }

}
