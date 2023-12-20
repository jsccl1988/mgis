#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(Shapefile, isValid)
{
    EXPECT_NO_THROW(ShapefilePtr(new Shapefile(gaiaAllocShapefile())));
}
