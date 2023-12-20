/**
 * @file    OutputBuffer.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main OutputBuffer class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/OutputBuffer.h"

namespace SpatiaLite
{

    OutputBuffer::OutputBuffer(OutputBufferType buffer) :
        Buffer<OutputBufferType>(buffer, OutputBuffer::clean)
    {
        gaiaOutBufferInitialize(this->get());
    }

    OutputBuffer::~OutputBuffer()
    {
        if (this->get())
        {
            gaiaOutBufferReset(this->get());
        }
    }

    void OutputBuffer::clean(OutputBufferType buffer)
    {
        if (buffer)
        {
            delete buffer;
        }
    }

    bool OutputBuffer::isValid() const
    {
        return (this->get() &&
                this->get()->Error == 0 &&
                this->get()->Buffer != 0);
    }

}
