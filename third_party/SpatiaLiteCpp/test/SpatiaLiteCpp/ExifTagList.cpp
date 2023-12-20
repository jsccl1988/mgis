#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(ExifTagList, isValid)
{
    const unsigned char blob = 0;
    EXPECT_NO_THROW(ExifTagListPtr tags(new ExifTagList(&blob, 1)));
}
