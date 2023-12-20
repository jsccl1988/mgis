#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

#ifndef SPATIALITECPP_TEST_EX_DIR
    #define SPATIALITECPP_TEST_EX_DIR "../../examples"
#endif
const std::string XMLDOCUMENT_EX_DIR = SPATIALITECPP_TEST_EX_DIR;

TEST(XmlDocument, isValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    EXPECT_NO_THROW(XmlDocumentPtr(XmlBlob::load(XMLDOCUMENT_EX_DIR + "/xml/books.xml")));
}
