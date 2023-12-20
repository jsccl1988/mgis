/**
 * @file    Checksum.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main Checksum class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/Checksum.h"

#include "SpatiaLiteCpp/Auxiliary.h"

extern "C"
{
#include "sqlite3.h"
#include "spatialite.h"
#include "spatialite/gaiaaux.h"
}

#include <cstdlib>
#include <stdexcept>

namespace SpatiaLite
{

    Checksum::Checksum(ChecksumType checksum) :
        Buffer<ChecksumType>(checksum, gaiaFreeMD5Checksum)
    {
    }

    std::string Checksum::finalize()
    {
        return Auxiliary::toString(gaiaFinalizeMD5Checksum(this->get()),
                                   "Error computing checksum");
    }

    void Checksum::update(const unsigned char * blob, int size)
    {
        gaiaUpdateMD5Checksum(this->get(), blob, size);
    }

}
