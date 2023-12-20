/**
 * @file    XmlBlob.cpp
 * @ingroup SpatiaLiteCpp
 * @brief   Main XmlBlob class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma warning(disable:4996)

#include "SpatiaLiteCpp/XmlBlob.h"

#include "SpatiaLiteCpp/Auxiliary.h"
#include "SpatiaLiteCpp/SpatialDatabase.h"
#include "SpatiaLiteCpp/XmlDocument.h"

extern "C"
{
#include "sqlite3.h"
#include "spatialite.h"
}

#include <fstream>
#include <stdexcept>

namespace SpatiaLite
{

    XmlBlob::XmlBlob(XmlBlobType blob, int size) :
        BufferSized<XmlBlobType>(blob, size, XmlBlob::clean)
    {
    }

    XmlBlob * XmlBlob::addFileId(SpatialDatabase const & database,
                                 const char * identifier,
                                 const char * namespaceId,
                                 const char * uriId,
                                 const char * nsCharstring,
                                 const char * uriCharstring) const
    {
        XmlBlobType newBlob = 0;
        int newSize = 0;
        int status = gaiaXmlBlobAddFileId(database.getCache(),
                                          this->get(),
                                          this->getSize(),
                                          identifier,
                                          namespaceId,
                                          uriId,
                                          nsCharstring,
                                          uriCharstring,
                                          &newBlob,
                                          &newSize);
        if (!status)
        {
            throw std::runtime_error("Failed to create add File ID!");
        }
        return new XmlBlob(newBlob, newSize);
    }

    XmlBlob * XmlBlob::addParentId(SpatialDatabase const & database,
                                   const char * identifier,
                                   const char * namespaceId,
                                   const char * uriId,
                                   const char * nsCharstring,
                                   const char * uriCharstring) const
    {
        XmlBlobType newBlob = 0;
        int newSize = 0;
        int status = gaiaXmlBlobAddParentId(database.getCache(),
                                            this->get(),
                                            this->getSize(),
                                            identifier,
                                            namespaceId,
                                            uriId,
                                            nsCharstring,
                                            uriCharstring,
                                            &newBlob,
                                            &newSize);
        if (!status)
        {
            throw std::runtime_error("Failed to create add Parent ID!");
        }
        return new XmlBlob(newBlob, newSize);
    }

    void XmlBlob::clean(XmlBlobType buffer)
    {
        if (buffer)
        {
            gaiaFree(buffer);
        }
    }

    XmlBlob * XmlBlob::compression(int compressed) const
    {
        XmlBlobType newBlob = 0;
        int newSize = 0;
        gaiaXmlBlobCompression(this->get(),
                               this->getSize(),
                               compressed,
                               &newBlob,
                               &newSize);
        if (!newBlob)
        {
            throw std::runtime_error("Failed to compress XML Blob!");
        }
        return new XmlBlob(newBlob, newSize);
    }

    XmlDocument * XmlBlob::fromBlob(int indent) const
    {
        int size = 0;
        XmlDocumentType buffer = 0;
        gaiaXmlFromBlob(this->get(), this->getSize(), indent, &buffer, &size);
        if (!buffer)
        {
            throw std::runtime_error("Failed to extract XML from Blob!");
        }
        return new XmlDocument(buffer, size);
    }

    std::string XmlBlob::getAbstract() const
    {
        return Auxiliary::toString(gaiaXmlBlobGetAbstract(this->get(),
                                                          this->getSize()),
                                "Failed to get XML BLOB abstract!");
    }

    std::string XmlBlob::getEncoding() const
    {
        return Auxiliary::toString(gaiaXmlBlobGetEncoding(this->get(),
                                                          this->getSize()),
                                "Failed to get XML BLOB encoding!");
    }

    std::string XmlBlob::getFileId() const
    {
        return Auxiliary::toString(gaiaXmlBlobGetFileId(this->get(),
                                                        this->getSize()),
                                "Failed to get XML BLOB file id!");
    }

    GeometryBuffer * XmlBlob::getGeometry() const
    {
        int size = 0;
        GeometryBufferType geometry = 0;
        gaiaXmlBlobGetGeometry(this->get(), this->getSize(), &geometry, &size);
        if (!geometry)
        {
            throw std::runtime_error("Failed to get geometry from Blob!");
        }
        return new GeometryBuffer(geometry, size, XmlBlob::clean);
    }

    std::string XmlBlob::getInternalSchemaURI(SpatialDatabase const & database,
                                              XmlDocument const & xml)
    {
        return Auxiliary::toString(gaiaXmlGetInternalSchemaURI(
                                       database.getCache(),
                                       xml.get(),
                                       xml.getSize()),
                                   "Failed to get internal schema URI!");
    }

    std::string XmlBlob::getLastParseError(SpatialDatabase const & database) const
    {
        return Auxiliary::toString(gaiaXmlBlobGetLastParseError(
                                       database.getCache()),
                                   "Failed to get the most recent XML Parse error!");
    }

    std::string XmlBlob::getLastValidateError(SpatialDatabase const & database) const
    {
        return Auxiliary::toString(gaiaXmlBlobGetLastValidateError(
                                       database.getCache()),
                                   "Failed to get the most recent XML Validate error!");
    }

    std::string XmlBlob::getLastXPathError(SpatialDatabase const & database) const
    {
        return Auxiliary::toString(gaiaXmlBlobGetLastXPathError(
                                       database.getCache()),
                                   "Failed to get the most recent XPath error!");
    }

    std::string XmlBlob::getParentId() const
    {
        return Auxiliary::toString(gaiaXmlBlobGetParentId(this->get(),
                                                          this->getSize()),
                                   "Failed to get XML BLOB parent id!");
    }

    std::string XmlBlob::getSchemaUri() const
    {
        return Auxiliary::toString(gaiaXmlBlobGetSchemaURI(this->get(),
                                                           this->getSize()),
                                   "Failed to get XML BLOB schema URI!");
    }

    std::string XmlBlob::getTitle() const
    {
        return Auxiliary::toString(gaiaXmlBlobGetTitle(this->get(),
                                                       this->getSize()),
                                   "Failed to get XML BLOB title!");
    }

    XmlDocument * XmlBlob::load(std::string const & path)
    {

        // Read file contents into array
        std::ifstream file(path.c_str(), std::ifstream::binary);
        std::filebuf * buffer = file.rdbuf();
        std::size_t size = buffer->pubseekoff (0, file.end, file.in);
        buffer->pubseekpos(0, file.in);
        char * blob = new char[size];
        buffer->sgetn(blob, size);
        file.close();

        return new XmlDocument((XmlDocumentType)blob,
                               (int)size,
                               XmlDocument::cleanDelete);

    }

    XmlBlob * XmlBlob::setFileId(SpatialDatabase const & database,
                                 const char * identifier) const
    {

        XmlBlobType blob = 0;
        int size = 0;
        int status = gaiaXmlBlobSetFileId(database.getCache(),
                                          this->get(),
                                          this->getSize(),
                                          identifier,
                                          &blob,
                                          &size);
        if (!status)
        {
            throw std::runtime_error("Failed to create set File ID!");
        }
        return new XmlBlob(blob, size);

    }

    XmlBlob * XmlBlob::setParentId(SpatialDatabase const & database,
                                   const char * identifier) const
    {

        XmlBlobType blob = 0;
        int size = 0;
        int status = gaiaXmlBlobSetParentId(database.getCache(),
                                            this->get(),
                                            this->getSize(),
                                            identifier,
                                            &blob,
                                            &size);
        if (!status)
        {
            throw std::runtime_error("Failed to create set Parent ID!");
        }
        return new XmlBlob(blob, size);

    }

    int XmlBlob::store(std::string const & path, int indent) const
    {
        return gaiaXmlStore(this->get(), this->getSize(), path.c_str(), indent);
    }

    std::string XmlBlob::textFromBlob(int indent) const
    {
        return Auxiliary::toString(gaiaXmlTextFromBlob(this->get(),
                                                       this->getSize(),
                                                       indent),
                                   "Failed to extract XML document from blob.");
    }

    XmlBlob * XmlBlob::toBlob(SpatialDatabase const & database,
                              XmlDocument const & xml,
                              int compressed,
                              const char * schemaURI,
                              bool parsing,
                              bool validation)
    {

        char * parsingErrors = 0;
        char * validationErrors = 0;

        char ** pParsing = (parsing ? &parsingErrors : 0);
        char ** pValidation = (validation ? &validationErrors : 0);

        // Extract Blob
        XmlBlobType blob = 0;
        int size = 0;
        gaiaXmlToBlob(database.getCache(),
                      xml.get(),
                      xml.getSize(),
                      compressed,
                      schemaURI,
                      &blob,
                      &size,
                      pParsing,
                      pValidation);

        // Check for errors
        if (parsingErrors)
        {
            throw std::runtime_error(Auxiliary::toString(parsingErrors,
                "Failed to parse Xml to Blob"));
        }
        if (validationErrors)
        {
            throw std::runtime_error(Auxiliary::toString(validationErrors,
                "Failed to validate Xml to Blob"));
        }
        if (!blob)
        {
            throw std::runtime_error("Failed to convert XML to Blob!");
        }

        return new XmlBlob(blob, size);

    }

    std::string XmlBlob::version()
    {
        return Auxiliary::toString(gaia_libxml2_version(),
                                   "Failed to get libxml2 version!");
    }

}
