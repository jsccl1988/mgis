/**
 * @file    Checksum.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main Checksum class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

#include "SpatiaLiteCpp/Buffer.hpp"

#include <string>

namespace SpatiaLite
{

    /**
     * Checksum buffer data type
     */
    typedef void * ChecksumType;

    /**
     * @brief RAII management of a checksum returned by gaiaCreateMD5Checksum.
     */
    class SPATIALITECPP_ABI Checksum : public Buffer<ChecksumType>
    {

    public:

        /**
         * @brief Takes ownership of an existing checksum pointer
         * @param[in] checksum Existing checksum pointer
         */
        explicit Checksum(ChecksumType checksum);

        /**
         * Finalize checksum and return final value.
         * @returns Checksum value as string.
         * @throws std::runtime_error on failure
         * @warning This resets checksum to initial state.
         */
        std::string finalize();

        /**
         * Update checksum. Can be called repeatedly.
         * @param[in] blob An arbitrary sequence of binary data
         * @param[in] size Size of blob
         */
        void update(const unsigned char * blob, int size);

    };

}
