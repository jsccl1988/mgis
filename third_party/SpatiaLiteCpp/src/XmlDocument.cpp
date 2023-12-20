/**
 * @file    XmlDocument.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main XmlDocument class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

#include "SpatiaLiteCpp/XmlDocument.h"

extern "C"
{
#include "sqlite3.h"
#include "spatialite.h"
}

namespace SpatiaLite
{

    XmlDocument::XmlDocument(XmlDocumentType xml,
                             int size,
                             void (*clean)(XmlDocumentType)) :
        BufferSized<XmlDocumentType>(xml, size, clean)
    {
    }

    void XmlDocument::cleanDelete(XmlDocumentType buffer)
    {
        if (buffer)
        {
            delete [] buffer;
        }
    }

    void XmlDocument::cleanGaia(XmlDocumentType buffer)
    {
        if (buffer)
        {
            gaiaFree(buffer);
        }
    }

}
