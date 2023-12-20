#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(Checksum, isValid)
{
    EXPECT_NO_THROW(ChecksumPtr(new Checksum(gaiaCreateMD5Checksum())));
}

TEST(Checksum, isUpdateValid)
{
    ChecksumPtr sum(new Checksum(gaiaCreateMD5Checksum()));
    unsigned char x = '0';
    EXPECT_NO_THROW(sum->update(&x, 1));
}

TEST(Checksum, isFinalizeValid)
{
    ChecksumPtr sum(new Checksum(gaiaCreateMD5Checksum()));
    unsigned char x = '0';
    sum->update(&x, 1);
    EXPECT_NO_THROW(sum->finalize());
}
