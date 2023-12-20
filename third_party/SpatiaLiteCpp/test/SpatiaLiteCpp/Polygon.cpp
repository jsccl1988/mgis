#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(Polygon, isValid)
{
    EXPECT_NO_THROW(PolygonPtr(new Polygon(gaiaAllocPolygon(5, 1))));
}
