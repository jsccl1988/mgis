/**
 * @file    Auxiliary.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main Auxiliary class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

#include "SpatiaLiteCpp/SpatiaLiteCppAbi.h"

extern "C"
{
#include "sqlite3.h"
#include "spatialite/gaiaaux.h"
}

#include <string>
#include <utility>

namespace SpatiaLite
{

    /**
     * @brief RAII management of auxillary functions.
     */
    class SPATIALITECPP_ABI Auxiliary
    {

    public:

        /**
         * Properly formats an SQL generic string (dequoting).
         * @param[in] value The string to be dequoted
         * @returns The formatted string
         * @throws std::runtime_error on failure
         */
        static std::string dequotedSql(const char * value);

        /**
         * Properly formats an SQL name.
         * @param[in] value The SQL name to be formatted
         * @returns The formatted string
         * @throws std::runtime_error on failure
         */
        static std::string doubleQuotedSql(const char * value);

        /**
         * Properly formats an SQL generic string.
         * @param[in] value The string to be formatted
         * @param[in] quote GAIA_SQL_SINGLE_QUOTE or GAIA_SQL_DOUBLE_QUOTE
         * @returns The formatted string:
         * @throws std::runtime_error on failure
         */
        static std::string quotedSql(const char * value, int quote);

        /**
         * Converts spatialite allocated char array to string
         * @param[in] value String to convert
         * @param[in] error Error message to throw on error
         * @returns Converted string
         * @throws std::runtime_error on failure
         * @warning Frees up allocated array with gaiaFree() and so is not safe
         *          to use after this call.
         */
        static std::string toString(char * value,
                                    std::string const & error);

    private:

        /**
         * @brief Private constructor for a static class
         */
        Auxiliary();

    };

}
