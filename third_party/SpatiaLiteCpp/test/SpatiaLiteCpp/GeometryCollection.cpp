#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(GeometryCollection, isBlobValid)
{
    BlobPtr point(Point::makePoint(4326, 0, 0));
    EXPECT_NO_THROW(GeometryCollectionPtr(new GeometryCollection(*point)));
}

TEST(GeometryCollection, isGeometryCollectionValid)
{
    EXPECT_NO_THROW(GeometryCollectionPtr(new GeometryCollection(gaiaAllocGeomColl())));
}

TEST(GeometryCollection, isSqliteColumnValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    db.getDatabase()->exec("SELECT InitSpatialMetadata(1)");
    db.getDatabase()->exec("CREATE TABLE test (PK INTEGER NOT NULL PRIMARY KEY)");
    db.getDatabase()->exec("SELECT AddGeometryColumn('test', 'geom', 4326, 'POINT', 2)");
    db.getDatabase()->exec("SELECT CreateSpatialIndex('test', 'geom')");
    db.getDatabase()->exec("INSERT INTO test (pk, geom) VALUES (NULL, GeomFromText('POINT(1.01 2.02)', 4326))");
    SQLite::Statement query(*db.getDatabase(), "SELECT 0 FROM test;");
    query.executeStep();
    EXPECT_NO_THROW(GeometryCollectionPtr(new GeometryCollection(query.getColumn(0))));
}
