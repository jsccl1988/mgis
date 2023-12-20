#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(Blob, isCompressedBlobWkbValid)
{
    BlobPtr point(Point::makePoint(4326, 0, 0));
    GeometryCollectionPtr point1(new GeometryCollection(*point));
    EXPECT_NO_THROW(BlobPtr(Blob::toCompressedBlobWkb(point1->get())));
}

TEST(Blob, isCompressedBlobWkbInvalid)
{
    EXPECT_THROW(BlobPtr(Blob::toCompressedBlobWkb(0)), std::runtime_error);
}

TEST(Blob, isFgfValid)
{
    BlobPtr point(Point::makePoint(4326, 0, 0));
    GeometryCollectionPtr point1(new GeometryCollection(*point));
    EXPECT_NO_THROW(BlobPtr(Blob::toFgf(point1->get(), GAIA_XY)));
}

TEST(Blob, isFgfInvalid)
{
    EXPECT_THROW(BlobPtr(Blob::toFgf(0, GAIA_XY)), std::runtime_error);
}

TEST(Blob, isHexWkbValid)
{
    BlobPtr point(Point::makePoint(4326, 0, 0));
    GeometryCollectionPtr point1(new GeometryCollection(*point));
    EXPECT_NO_THROW(BlobPtr(Blob::toHexWkb(point1->get())));
}

TEST(Blob, isHexWkbInvalid)
{
    EXPECT_THROW(BlobPtr(Blob::toHexWkb(0)), std::runtime_error);
}

TEST(Blob, isSpatiaLiteBlobWkbValid)
{
    BlobPtr point(Point::makePoint(4326, 0, 0));
    GeometryCollectionPtr point1(new GeometryCollection(*point));
    EXPECT_NO_THROW(BlobPtr(Blob::toSpatiaLiteBlobWkb(point1->get())));
}

TEST(Blob, isSpatiaLiteBlobWkbInvalid)
{
    EXPECT_THROW(BlobPtr(Blob::toSpatiaLiteBlobWkb(0)), std::runtime_error);
}

TEST(Blob, isWkbValid)
{
    BlobPtr point(Point::makePoint(4326, 0, 0));
    GeometryCollectionPtr point1(new GeometryCollection(*point));
    EXPECT_NO_THROW(BlobPtr(Blob::toWkb(point1->get())));
}

TEST(Blob, isWkbInvalid)
{
    EXPECT_THROW(BlobPtr(Blob::toWkb(0)), std::runtime_error);
}
