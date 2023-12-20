/**
 * @file    OutputBuffer.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main Output buffer class.
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
     * Output buffer buffer data type
     */
    typedef gaiaOutBufferPtr OutputBufferType;

    /**
     * @brief RAII management of a gaiaOutBufferPtr.
     */
    class SPATIALITECPP_ABI OutputBuffer : public Buffer<OutputBufferType>
    {

    public:

        /**
         * @brief Create new buffer
         * @param buffer Source buffer
         */
        explicit OutputBuffer(OutputBufferType buffer);

        /**
         * @brief Clean up buffer
         */
        ~OutputBuffer();

        /**
         * @returns True if writing buffer is valid otherwise false.
         */
        bool isValid() const;

    private:

        /**
         * Cleans up OutputBuffer array memory
         * @param[in] buffer Output buffer.
         */
        static void clean(OutputBufferType buffer);

    };

}
