/**
 * @file    XmlBlob.h
 * @ingroup SpatiaLiteCpp
 * @brief   Main XmlBlob class.
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */
#pragma once

#include "SpatiaLiteCpp/Buffer.hpp"

#include <string>

namespace SpatiaLite
{

    // Forward declarations
    class SpatialDatabase;
    class XmlDocument;

    /**
     * Geometry buffer data type
     */
    typedef unsigned char * GeometryBufferType;

    /**
     * Geometry buffer extracted from XML Blob
     */
    typedef BufferSized<GeometryBufferType> GeometryBuffer;

    /**
     * XML Blob buffer data type
     */
    typedef unsigned char * XmlBlobType;

    /**
     * @brief RAII management of a XML BLOB array.
     */
    class SPATIALITECPP_ABI XmlBlob : BufferSized<XmlBlobType>
    {

    public:

        /**
         * @brief Takes ownership of an existing BLOB pointer
         * @param[in] blob Source blob buffer
         * @param[in] size Blob buffer size
         */
        XmlBlob(XmlBlobType blob, int size);

        /**
         * @brief Creates a new XmlBlob by inserting a FileId value.
         * @param[in] database      Current Spatial Database
         * @param[in] identifier    The new FileId value to be inserted.
         * @param[in] namespaceId   Prefix corresponding to FileIdentifier
         *                          NameSpace (may be NULL)
         * @param[in] uriId         URI corresponding to the FileIdentifier
         *                          NameSpace (may be NULL)
         * @param[in] nsCharstring  Prefix corresponding to CharacterString
         *                          NameSpace (may be NULL)
         * @param[in] uriCharstring URI corresponding to CharacterString
         *                          NameSpace (may be NULL)
         * @returns New XmlBlob pointer
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer. Should be owned by a XmlBlobPtr.
         */
        XmlBlob * addFileId(SpatialDatabase const & database,
                            const char * identifier,
                            const char * namespaceId = 0,
                            const char * uriId = 0,
                            const char * nsCharstring = 0,
                            const char * uriCharstring = 0) const;

        /**
         * @brief Creates a new XmlBlob by inserting a ParentId value.
         * @param[in]  database      Current Spatial Database
         * @param[in]  identifier    The new ParentId value to be inserted.
         * @param[in]  namespaceId   Prefix corresponding to FileIdentifier
         *                           NameSpace (may be NULL)
         * @param[in]  uriId         URI corresponding to the FileIdentifier
         *                           NameSpace (may be NULL)
         * @param[in]  nsCharstring  Prefix corresponding to CharacterString
         *                           NameSpace (may be NULL)
         * @param[in]  uriCharstring URI corresponding to CharacterString
         *                           NameSpace (may be NULL)
         * @returns New XmlBlob pointer
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer. Should be owned by a XmlBlobPtr.
         */
        XmlBlob * addParentId(SpatialDatabase const & database,
                              const char * identifier,
                              const char * namespaceId = 0,
                              const char * uriId = 0,
                              const char * nsCharstring = 0,
                              const char * uriCharstring = 0) const;

        /**
         * @brief Cleans up XML Blob buffer array memory
         * @param[in] buffer XML Blob buffer.
         */
        static void clean(XmlBlobType buffer);

        /**
         * @brief Creates another XmlBlob buffer compressed / uncompressed.
         * @param[in] compressed If 1 the XmlBlob will be zip-compressed.
         * @returns New XmlBlob pointer
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer. Should be owned by a XmlBlobPtr.
         */
        XmlBlob * compression(int compressed) const;

        /**
         * @brief Creates an XML Document from within an XmlBLOB buffer.
         * @param[in] indent If a negative value is passed the XMLDocument will
         *                   be extracted exactly as it was when loaded.
         *                   Otherwise it will be properly formatted using the
         *                   required intenting (max. 8); ZERO means that the
         *                   whole XML Document will consist of a single line.
         * @returns New Buffer pointer
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer.
         *          Should be owned by a XmlDocumentPtr.
         */
        XmlDocument * fromBlob(int indent = 0) const;

        /**
         * @brief Returns the Abstract from a valid XmlBlob buffer.
         * @returns The Abstract from a valid XmlBlob buffer.
         * @throws std::runtime_error on failure
         */
        std::string getAbstract() const;

        /**
         * @brief Returns the Charset Encoding from a valid XmlBlob buffer.
         * @returns The Charset Encoding from a valid XmlBlob buffer.
         * @throws std::runtime_error on failure
         */
        std::string getEncoding() const;

        /**
         * @brief Returns the FileIdentifier from a valid XmlBLOB buffer.
         * @returns The FileIdentifier from a valid XmlBLOB buffer.
         * @throws std::runtime_error on failure
         */
        std::string getFileId() const;

        /**
         * @returns A new geometry buffer from a valid XmlBLOB buffer.
         * @throws std::runtime_error on failure
         * @warning Caller must call delete.
         */
        GeometryBuffer * getGeometry() const;

        /**
         * @brief The most recent XML Parse error/warning (if any).
         * @param[in] database Current Spatial Database
         * @param[in] xml      XML Document buffer
         * @throws std::runtime_error on failure
         */
        static std::string getInternalSchemaURI(SpatialDatabase const & database,
                                                XmlDocument const & xml);

        /**
         * @brief The most recent XML Parse error/warning (if any).
         * @param[in] database Current Spatial Database
         * @returns Error/warning string
         * @throws std::runtime_error on failure
         */
        std::string getLastParseError(SpatialDatabase const & database) const;

        /**
         * @brief The most recent XML Validate error/warning (if any).
         * @param[in] database Current Spatial Database
         * @returns Error/warning string
         * @throws std::runtime_error on failure
         */
        std::string getLastValidateError(SpatialDatabase const & database) const;

        /**
         * @brief The most recent XPath error/warning (if any).
         * @param[in] database Current Spatial Database
         * @returns Error/warning string
         * @throws std::runtime_error on failure
         */
        std::string getLastXPathError(SpatialDatabase const & database) const;

        /**
         * @brief Returns the Parent ID from a valid XmlBLOB buffer.
         * @returns The Parent ID from a valid XmlBLOB buffer.
         * @throws std::runtime_error on failure
         */
        std::string getParentId() const;

        /**
         * @brief Returns the Schema URI from a valid XmlBLOB buffer.
         * @returns The Schema URI from a valid XmlBLOB buffer.
         * @throws std::runtime_error on failure
         */
        std::string getSchemaUri() const;

        /**
         * @brief Returns the Title from a valid XmlBLOB buffer.
         * @returns The Title from a valid XmlBLOB buffer.
         * @throws std::runtime_error on failure
         */
        std::string getTitle() const;

        /**
         * @brief Load an external XML Document.
         * @param[in] path Path or URL to the external XML Document
         * @returns New Buffer pointer
         * @warning Caller must delete pointer.
         *          Should be owned by a XmlDocumentPtr.
         */
        static XmlDocument * load(std::string const & path);

        /**
         * @brief Creates a new XmlBlob by replacing the FileId value.
         * @param[in] database   Current Spatial Database
         * @param[in] identifier The new FileId value to be inserted.
         * @returns New XmlBlob pointer
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer. Should be owned by a XmlBlobPtr.
         */
        XmlBlob * setFileId(SpatialDatabase const & database,
                            const char * identifier) const;

        /**
         * @brief Creates a new XmlBlob by replacing the ParentId value.
         * @param[in] database   Current Spatial Database
         * @param[in] identifier The new FileId value to be inserted.
         * @returns New XmlBlob pointer
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer. Should be owned by a XmlBlobPtr.
         */
        XmlBlob * setParentId(SpatialDatabase const & database,
                              const char * identifier) const;

        /**
         * @brief Store an external XML Document.
         * @param[in] path   Path or URL to the external XML Document
         * @param[in] indent If a negative value is passed the XML Document
         *                   will be extracted exactly as it was when loaded.
         *                   Otherwise it will be properly formatted using the
         *                   required intenting (max. 8); ZERO means that the
         *                   whole XML Document will consist of a single line.
         * @returns 0 on failure otherwise success
         */
        int store(std::string const & path, int indent = 0) const;

        /**
         * @brief Extract an XMLDocument from within an XmlBLOB buffer.
         * @param[in] indent If a negative value is passed the XML Document
         *                   will be extracted exactly as it was when loaded.
         *                   Otherwise it will be properly formatted using the
         *                   required intenting (max. 8); ZERO means that the
         *                   whole XML Document will consist of a single line.
         * @throws std::runtime_error on failure
         */
        std::string textFromBlob(int indent = 0) const;

        /**
         * @brief Creates an Xml Blob buffer from a XML Document.
         * @param[in] database   Current Spatial Database
         * @param[in] xml        XML Document buffer
         * @param[in] compressed If TRUE the returned XmlBLOB will be
         *                       zip-compressed.
         * @param[in] schemaURI  If not NULL the XML document will be assumed
         *                       to be valid only if it successfully passes a
         *                       formal Schema valitadion.
         * @param[in] parsing    If true parsing errors will throw exception
         * @param[in] validation If true schema validation errors will throw
         *                       exception
         * @throws std::runtime_error on failure
         * @warning Caller must delete pointer. Should be owned by a XmlBlobPtr.
        */
        static XmlBlob * toBlob(SpatialDatabase const & database,
                                XmlDocument const & xml,
                                int compressed = 0,
                                const char * schemaURI = 0,
                                bool parsing = true,
                                bool validation = true);

        /**
         * @brief Retuns the LIBXML2 version
         * @returns LIBXML2 version string
         * @throws std::runtime_error on failure
         */
        static std::string version();

    };

}
