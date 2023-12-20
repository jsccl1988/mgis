/**
 * @file    Converter.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main UTF8 Converter class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

#include "SpatiaLiteCpp/Buffer.hpp"

#include <string>

namespace SpatiaLite
{

    /**
     * Converter buffer data type
     */
    typedef void * ConverterType;

    /**
     * @brief RAII management of a UTF8 converter from gaiaCreateUTF8Converter.
     */
    class SPATIALITECPP_ABI Converter : public Buffer<ConverterType>
    {

    public:

        /**
         * @brief Creates a converter
         * @param[in] charset the GNU ICONV name identifying the input charset
         */
        explicit Converter(const char * charset);

        /**
         * Convert a text string to UTF8
         * @param[in] buffer Input text string
         * @param[in] size Sizeof buffer (in bytes)
         * @returns null-terminated UTF8 encoded string
         * @throws std::runtime_error on failure
         */
        std::string convert(const char * buffer, int size) const;

    };

}
