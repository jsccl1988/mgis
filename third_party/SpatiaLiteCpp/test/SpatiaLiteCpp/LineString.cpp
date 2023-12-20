#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(LineString, isValid)
{
    EXPECT_NO_THROW(LineStringPtr(new LineString(gaiaAllocLinestring(5))));
}

TEST(LineString, isMakeLineValid)
{
    double z = 0;
    BlobPtr point(Point::makePoint(4326, 0, 0));
    BlobPtr pointZ(Point::makePoint(4326, 0, 0, &z));
    GeometryCollectionPtr point1(new GeometryCollection(*point));
    GeometryCollectionPtr point2(new GeometryCollection(*pointZ));
    EXPECT_NO_THROW(BlobPtr(LineString::makeLine(point1->get(), point2->get())));
}

TEST(LineString, isMakeLineGeometryOneInvalid)
{
    double z = 0;
    BlobPtr pointZ(Point::makePoint(4326, 0, 0, &z));
    GeometryCollectionPtr point2(new GeometryCollection(*pointZ));
    EXPECT_THROW(BlobPtr(LineString::makeLine(0, point2->get())), std::runtime_error);
}

TEST(LineString, isMakeLineGeometryTwoInvalid)
{
    BlobPtr point(Point::makePoint(4326, 0, 0));
    GeometryCollectionPtr point1(new GeometryCollection(*point));
    EXPECT_THROW(BlobPtr(LineString::makeLine(point1->get(), 0)), std::runtime_error);
}
