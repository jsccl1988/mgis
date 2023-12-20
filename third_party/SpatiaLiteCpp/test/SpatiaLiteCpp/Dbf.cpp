#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(Dbf, isValid)
{
    EXPECT_NO_THROW(DbfPtr(new Dbf(gaiaAllocDbf())));
}
