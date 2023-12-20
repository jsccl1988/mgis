#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(DbfList, isValid)
{
    EXPECT_NO_THROW(DbfListPtr(new DbfList(gaiaAllocDbfList())));
}
