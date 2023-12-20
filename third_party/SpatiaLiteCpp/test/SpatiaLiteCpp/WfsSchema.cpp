#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

#ifndef SPATIALITECPP_TEST_EX_DIR
    #define SPATIALITECPP_TEST_EX_DIR "../../examples"
#endif
const std::string WFSSCHEMA_EX_DIR = SPATIALITECPP_TEST_EX_DIR;

TEST(WfsSchema, isInvalid)
{
    EXPECT_THROW(WfsSchemaPtr(WfsSchema::create("", "")), std::runtime_error);
}

TEST(WfsSchema, isValid)
{
    EXPECT_NO_THROW(WfsSchemaPtr(WfsSchema::create(WFSSCHEMA_EX_DIR + "/wfs/testDescribeFeatureType.wfs", "sf:roads")));
}

TEST(WfsSchema, isCountValid)
{
    WfsSchemaPtr schema(WfsSchema::create(WFSSCHEMA_EX_DIR + "/wfs/testDescribeFeatureType.wfs", "sf:roads"));
    int count = get_wfs_schema_column_count(schema->get());
    EXPECT_EQ(count, 24);
}
