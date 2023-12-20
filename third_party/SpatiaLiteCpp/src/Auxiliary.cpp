/**
 * @file    Auxiliary.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main Auxiliary class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/Auxiliary.h"

#include "SpatiaLiteCpp/Buffer.hpp"

extern "C"
{
#include "spatialite.h"
}

#include <stdexcept>

namespace SpatiaLite
{

    Auxiliary::Auxiliary()
    {
    }

    std::string Auxiliary::dequotedSql(const char * value)
    {
        return Auxiliary::toString(gaiaDequotedSql(value),
                                   "Failed dequoting SQL.");
    }

    std::string Auxiliary::doubleQuotedSql(const char * value)
    {
        return Auxiliary::toString(gaiaDoubleQuotedSql(value),
                                   "Failed doule quoting SQL.");
    }

    std::string Auxiliary::quotedSql(const char * value, int quote)
    {
        return Auxiliary::toString(gaiaQuotedSql(value, quote),
                                   "Failed quoting SQL.");
    }

    std::string Auxiliary::toString(char * value,
                                    std::string const & error)
    {
        if (value == 0) throw std::runtime_error(error);
        std::string converted(value);
        gaiaFree(value);
        return converted;
    }

}

