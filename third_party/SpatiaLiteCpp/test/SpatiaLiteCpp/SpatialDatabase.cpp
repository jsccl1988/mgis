#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(SpatialDatabase, isValid)
{
    EXPECT_NO_THROW(SpatialDatabase(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
}

TEST(SpatialDatabase, isCheckGeometriesValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    db.getDatabase()->exec("SELECT InitSpatialMetadata(1)");
    db.getDatabase()->exec("CREATE TABLE test (PK INTEGER NOT NULL PRIMARY KEY)");
    db.getDatabase()->exec("SELECT AddGeometryColumn('test', 'geom', 4326, 'POINT', 2)");
    db.getDatabase()->exec("SELECT CreateSpatialIndex('test', 'geom')");
    db.getDatabase()->exec("INSERT INTO test (pk, geom) VALUES (NULL, GeomFromText('POINT(1.01 2.02)', 4326))");
    int numInvalids = 0;
    std::string error;
    EXPECT_NE(db.checkGeometries(".", &numInvalids, error), 0);
    EXPECT_EQ(numInvalids, 0);
    EXPECT_EQ(error, "");
}

TEST(SpatialDatabase, isCheckGeometryValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    db.getDatabase()->exec("SELECT InitSpatialMetadata(1)");
    db.getDatabase()->exec("CREATE TABLE test (PK INTEGER NOT NULL PRIMARY KEY)");
    db.getDatabase()->exec("SELECT AddGeometryColumn('test', 'geom', 4326, 'POINT', 2)");
    db.getDatabase()->exec("SELECT CreateSpatialIndex('test', 'geom')");
    db.getDatabase()->exec("INSERT INTO test (pk, geom) VALUES (NULL, GeomFromText('POINT(1.01 2.02)', 4326))");
    int numRows = 0;
    int numInvalids = 0;
    std::string error;
    EXPECT_NE(db.checkGeometry("test", "geom", "report.html", &numRows, &numInvalids, error), 0);
    EXPECT_EQ(numRows, 1);
    EXPECT_EQ(numInvalids, 0);
    EXPECT_EQ(error, "");
}

TEST(SpatialDatabase, isCacheValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    EXPECT_TRUE(db.getCache() != 0);
}

TEST(SpatialDatabase, isCountValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    db.getDatabase()->exec("SELECT InitSpatialMetadata(1)");
    db.getDatabase()->exec("CREATE TABLE test (PK INTEGER NOT NULL PRIMARY KEY)");
    db.getDatabase()->exec("SELECT AddGeometryColumn('test', 'geom', 4326, 'POINT', 2)");
    db.getDatabase()->exec("SELECT CreateSpatialIndex('test', 'geom')");
    EXPECT_EQ(db.getCount("test"), 0);
    db.getDatabase()->exec("INSERT INTO test (pk, geom) VALUES (NULL, GeomFromText('POINT(1.01 2.02)', 4326))");
    EXPECT_EQ(db.getCount("test"), 1);
}

TEST(SpatialDatabase, isDatabaseValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    EXPECT_TRUE(db.getDatabase() != 0);
}

TEST(SpatialDatabase, isGeometryNameValid)
{
    EXPECT_EQ(SpatialDatabase::getGeometryName(GAIA_POINT), "POINT");
    EXPECT_EQ(SpatialDatabase::getGeometryName(GAIA_LINESTRING), "LINESTRING");
    EXPECT_EQ(SpatialDatabase::getGeometryName(GAIA_POLYGON), "POLYGON");
    EXPECT_EQ(SpatialDatabase::getGeometryName(GAIA_MULTIPOINT), "MULTIPOINT");
    EXPECT_EQ(SpatialDatabase::getGeometryName(GAIA_MULTILINESTRING), "MULTILINESTRING");
    EXPECT_EQ(SpatialDatabase::getGeometryName(GAIA_MULTIPOLYGON), "MULTIPOLYGON");
    EXPECT_EQ(SpatialDatabase::getGeometryName(GAIA_GEOMETRYCOLLECTION), "GEOMETRYCOLLECTION");
    EXPECT_EQ(SpatialDatabase::getGeometryName(-1), "EMPTY / NULL GEOMETRY");
}

TEST(SpatialDatabase, isHeadersValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    db.getDatabase()->exec("SELECT InitSpatialMetadata(1)");
    db.getDatabase()->exec("CREATE TABLE test (PK INTEGER NOT NULL PRIMARY KEY)");
    db.getDatabase()->exec("SELECT AddGeometryColumn('test', 'geom', 4326, 'POINT', 2)");
    db.getDatabase()->exec("SELECT CreateSpatialIndex('test', 'geom')");
    std::vector<std::string> headers = db.getHeaders("test");
    EXPECT_EQ(headers.size(), 2);
    EXPECT_EQ(headers.at(0), "PK");
    EXPECT_EQ(headers.at(1), "geom");
}

TEST(SpatialDatabase, isTypesValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    db.getDatabase()->exec("SELECT InitSpatialMetadata(1)");
    db.getDatabase()->exec("CREATE TABLE test (PK INTEGER NOT NULL PRIMARY KEY)");
    db.getDatabase()->exec("SELECT AddGeometryColumn('test', 'geom', 4326, 'POINT', 2)");
    db.getDatabase()->exec("SELECT CreateSpatialIndex('test', 'geom')");
    std::vector<std::string> types = db.getTypes("test");
    EXPECT_EQ(types.size(), 2);
    EXPECT_EQ(types.at(0), "INTEGER");
    EXPECT_EQ(types.at(1), "POINT");
}

TEST(SpatialDatabase, isSanitizeGeometriesValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    db.getDatabase()->exec("SELECT InitSpatialMetadata(1)");
    db.getDatabase()->exec("CREATE TABLE test (PK INTEGER NOT NULL PRIMARY KEY)");
    db.getDatabase()->exec("SELECT AddGeometryColumn('test', 'geom', 4326, 'POINT', 2)");
    db.getDatabase()->exec("SELECT CreateSpatialIndex('test', 'geom')");
    db.getDatabase()->exec("INSERT INTO test (pk, geom) VALUES (NULL, GeomFromText('POINT(1.01 2.02)', 4326))");
    int numNotRepaired = 0;
    std::string error;
    db.sanitizeGeometries("sanitize", ".", &numNotRepaired, error);
    EXPECT_EQ(numNotRepaired, 0);
}

TEST(SpatialDatabase, isSanitizeGeometryValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    db.getDatabase()->exec("SELECT InitSpatialMetadata(1)");
    db.getDatabase()->exec("CREATE TABLE test (PK INTEGER NOT NULL PRIMARY KEY)");
    db.getDatabase()->exec("SELECT AddGeometryColumn('test', 'geom', 4326, 'POINT', 2)");
    db.getDatabase()->exec("SELECT CreateSpatialIndex('test', 'geom')");
    db.getDatabase()->exec("INSERT INTO test (pk, geom) VALUES (NULL, GeomFromText('POINT(1.01 2.02)', 4326))");
    int numInvalids = 0;
    int numRepaired = 0;
    int numDiscarded = 0;
    int numFailures = 0;
    std::string error;
    db.sanitizeGeometry("test", "geom", "temp", "report.sanitize",
        &numInvalids, &numRepaired, &numDiscarded, &numFailures, error);
    EXPECT_EQ(numInvalids, 0);
    EXPECT_EQ(numRepaired, 0);
    EXPECT_EQ(numDiscarded, 0);
    EXPECT_EQ(numFailures, 0);
}
