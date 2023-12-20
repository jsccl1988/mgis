#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

#ifndef SPATIALITECPP_TEST_EX_DIR
    #define SPATIALITECPP_TEST_EX_DIR "../../examples"
#endif
const std::string WFSCATALOG_EX_DIR = SPATIALITECPP_TEST_EX_DIR;

TEST(WfsCatalog, isInvalid)
{
    EXPECT_THROW(WfsCatalogPtr(WfsCatalog::create("")), std::runtime_error);
}

TEST(WfsCatalog, isValid)
{
    const std::string directory = WFSCATALOG_EX_DIR + "/wfs/getcapabilities-1.0.0.wfs";
    EXPECT_NO_THROW(WfsCatalogPtr(WfsCatalog::create(directory.c_str())));
}

TEST(WfsCatalog, isLoadValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    db.getDatabase()->exec("SELECT InitSpatialMetadata(1);");
    std::string path    = WFSCATALOG_EX_DIR + "/wfs/test.wfs";
    std::string uri     = WFSCATALOG_EX_DIR + "/wfs/testDescribeFeatureType.wfs";
    std::string layer   = "topp:p02";
    int swap            = 0;
    std::string table   = "test_wfs1";
    std::string primary = "objectid";
    int spatial         = 1;
    int rows            = 0;
    EXPECT_NO_THROW(WfsCatalog::load(db, path.c_str(), uri.c_str(), layer.c_str(), swap, table.c_str(), primary.c_str(), spatial, &rows, 0, 0));
    EXPECT_NE(rows, 0);
}

TEST(WfsCatalog, isPagedValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    db.getDatabase()->exec("SELECT InitSpatialMetadata(1);");
    std::string path    = WFSCATALOG_EX_DIR + "/wfs/test.wfs";
    std::string uri     = WFSCATALOG_EX_DIR + "/wfs/testDescribeFeatureType.wfs";
    std::string layer   = "topp:p02";
    int swap            = 0;
    std::string table   = "test_wfs1";
    std::string primary = "objectid";
    int spatial         = 1;
    int rows            = 0;
    EXPECT_NO_THROW(WfsCatalog::paged(db, path.c_str(), uri.c_str(), layer.c_str(), swap, table.c_str(), primary.c_str(), spatial, 0, &rows, 0, 0));
    EXPECT_NE(rows, 0);
}
