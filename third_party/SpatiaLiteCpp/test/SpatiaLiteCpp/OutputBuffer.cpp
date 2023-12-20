#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(OutputBuffer, isValid)
{
    gaiaOutBufferPtr buf = new gaiaOutBuffer();
    gaiaOutBufferInitialize(buf);
    EXPECT_NO_THROW(OutputBufferPtr(new OutputBuffer(buf)));
}

TEST(OutputBuffer, isValidValid)
{
    GeometryCollectionPtr collection(new GeometryCollection(gaiaAllocGeomColl()));
    gaiaAddPointToGeomColl(collection->get(), 0.0, 0.0);
    OutputBufferPtr buffer(new OutputBuffer(new gaiaOutBuffer()));
    gaiaOutWkt(buffer->get(), collection->get());
    EXPECT_TRUE(buffer->isValid());
}
