/**
 * @file    Buffer.hpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main Buffer class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

#include "SpatiaLiteCpp/SpatiaLiteCppAbi.h"

namespace SpatiaLite
{

    /**
     * @brief RAII management of a buffer array.
     */
    template <class T>
    class Buffer
    {

    public:

        /**
         * @brief Takes ownership of an existing buffer
         * @param[in] buffer Existing buffer
         * @param[in] clean Buffer memory deallocator.
         */
        Buffer(T buffer, void (*clean)(T)) :
            _buffer(buffer),
            _clean(clean)
        {
        }

        /**
         * @brief Clean up pointer
         */
        virtual ~Buffer()
        {
            if (this->get() && this->_clean)
            {
                (this->_clean)(this->get());
            }
        }

        /**
         * @returns Raw pointer
         */
        T get() const
        {
            return this->_buffer;
        }

    private:

        /**
         * @brief Raw pointer
         */
        T _buffer;

        /**
         * @brief Deallocator
         */
        void (*_clean)(T);

    };

    /**
     * @brief RAII management of a sized buffer array.
     */
    template <class T>
    class BufferSized : public Buffer<T>
    {

    public:

        /**
         * @brief Takes ownership of an existing buffer
         * @param[in] buffer Existing buffer
         * @param[in] size Buffer size
         * @param[in] clean Buffer memory deallocator.
         */
        BufferSized(T buffer, int size, void (*clean)(T)) :
            Buffer<T>(buffer, clean),
            _size(size)
        {
        }

        /**
         * @returns Buffer size
         */
        int getSize() const
        {
            return this->_size;
        }

    protected:

        /**
         * @brief Buffer size
         */
        int _size;

    };

}
