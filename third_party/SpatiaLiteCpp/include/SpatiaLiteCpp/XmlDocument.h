/**
 * @file    XmlDocument.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main XmlDocument class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

#include "SpatiaLiteCpp/Buffer.hpp"

namespace SpatiaLite
{

    /**
     * XML Document buffer data type
     */
    typedef unsigned char * XmlDocumentType;

    /**
     * @brief RAII management of a XML Document.
     */
    class SPATIALITECPP_ABI XmlDocument : public BufferSized<XmlDocumentType>
    {

    public:

        /**
         * @brief Creates a XML Document.
         * @param[in] xml   Source XML Document
         * @param[in] size  Source XML Document size
         * @param[in] clean Memory deallocator. (Default = cleanGaia)
         */
        XmlDocument(XmlDocumentType xml,
                    int size,
                    void (*clean)(XmlDocumentType) = XmlDocument::cleanGaia);

        /**
         * Cleans up XML Document allocated by SpatiaLiteCpp. Calls delete [].
         * @param[in] xml XML Document.
         */
        static void cleanDelete(XmlDocumentType xml);

        /**
         * Cleans up XML Document allocated by SpatiaLite. Calls gaiaFree.
         * @param[in] xml XML Document.
         */
        static void cleanGaia(XmlDocumentType xml);

    };

}
