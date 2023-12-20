#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(DbfField, isValid)
{
    char name[] = "name";
    EXPECT_NO_THROW(DbfFieldPtr dbf(new DbfField(gaiaAllocDbfField(name,'C',0,1,0))));
}
