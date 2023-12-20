#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(VectorLayersList, isValid)
{
    SpatialDatabase db(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    db.getDatabase()->exec("SELECT InitSpatialMetadata(1)");
    db.getDatabase()->exec("CREATE TABLE test (PK INTEGER NOT NULL PRIMARY KEY)");
    db.getDatabase()->exec("SELECT AddGeometryColumn('test', 'geom', 4326, 'POINT', 2)");
    db.getDatabase()->exec("SELECT CreateSpatialIndex('test', 'geom')");
    db.getDatabase()->exec("INSERT INTO test (pk, geom) VALUES (NULL, GeomFromText('POINT(1.01 2.02)', 4326))");
    gaiaVectorLayersListPtr pl = gaiaGetVectorLayersList(
        db.getDatabase()->getHandle(), 0, 0, GAIA_VECTORS_LIST_FAST);
    EXPECT_NO_THROW(VectorLayersListPtr(new VectorLayersList(pl)));
}
