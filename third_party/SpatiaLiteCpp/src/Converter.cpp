/**
 * @file    Converter.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main UTF8 Converter class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/Converter.h"

#include "SpatiaLiteCpp/Auxiliary.h"

extern "C"
{
#include "sqlite3.h"
#include "spatialite.h"
#include "spatialite/gaiaaux.h"
}

namespace SpatiaLite
{

    Converter::Converter(const char * charset) :
        Buffer<ConverterType>(gaiaCreateUTF8Converter(charset),
                              gaiaFreeUTF8Converter)
    {
    }

    std::string Converter::convert(const char * buffer, int size) const
    {
        int error = 0;
        return Auxiliary::toString(gaiaConvertToUTF8(this->get(),
                                                     buffer,
                                                     size,
                                                     &error),
                                   "Error converting to UTF8");
    }

}
