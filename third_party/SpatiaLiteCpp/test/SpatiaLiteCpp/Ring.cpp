#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(Ring, isValid)
{
    EXPECT_NO_THROW(RingPtr ring(new Ring(gaiaAllocRing(5))));
}
