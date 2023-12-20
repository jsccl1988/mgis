#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(DynamicLine, isValid)
{
    EXPECT_NO_THROW(DynamicLinePtr(new DynamicLine(gaiaAllocDynamicLine())));
}
