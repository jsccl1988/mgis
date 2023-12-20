#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(Point, isValid)
{
    EXPECT_NO_THROW(PointPtr(new Point(gaiaAllocPoint(0,0))));
}

TEST(Point, isMakePointValid)
{
    EXPECT_NO_THROW(BlobPtr(Point::makePoint(4326, 0, 0)));
}
