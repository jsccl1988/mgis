#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

#ifndef SPATIALITECPP_TEST_EX_DIR
    #define SPATIALITECPP_TEST_EX_DIR "../../examples"
#endif
const std::string XMLBLOB_EX_DIR = SPATIALITECPP_TEST_EX_DIR;

TEST(XmlBlob, isValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    XmlDocumentPtr xml(XmlBlob::load(XMLBLOB_EX_DIR + "/xml/books.xml"));
    const std::string schema = XMLBLOB_EX_DIR + "/xml/books.xsd";
    EXPECT_NO_THROW(XmlBlobPtr(XmlBlob::toBlob(db, *xml, 1, schema.c_str())));
}

TEST(XmlBlob, isXmlBlobValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    XmlDocumentPtr xml(XmlBlob::load(XMLBLOB_EX_DIR + "/xml/books.xml"));
    const std::string schema = XMLBLOB_EX_DIR + "/xml/books.xsd";
    XmlBlobPtr blob(XmlBlob::toBlob(db, *xml, 1, schema.c_str()));
    EXPECT_NO_THROW(XmlBlobPtr(blob->compression(1)));
    EXPECT_NO_THROW(XmlDocumentPtr(blob->fromBlob(4)));
    EXPECT_THROW(blob->getAbstract(), std::runtime_error);
    EXPECT_NO_THROW(blob->getEncoding());
    EXPECT_THROW(blob->getFileId(), std::runtime_error);
    EXPECT_THROW(GeometryBufferPtr(blob->getGeometry()), std::runtime_error);
    EXPECT_NO_THROW(XmlBlob::getInternalSchemaURI(db, *xml));
    EXPECT_THROW(blob->getLastParseError(db), std::runtime_error);
    EXPECT_THROW(blob->getLastValidateError(db), std::runtime_error);
    EXPECT_THROW(blob->getLastXPathError(db), std::runtime_error);
    EXPECT_THROW(blob->getParentId(), std::runtime_error);
    EXPECT_NO_THROW(blob->getSchemaUri());
    EXPECT_THROW(blob->getTitle(), std::runtime_error);
    EXPECT_NO_THROW(blob->textFromBlob(4));
    EXPECT_NO_THROW(blob->version());

}
