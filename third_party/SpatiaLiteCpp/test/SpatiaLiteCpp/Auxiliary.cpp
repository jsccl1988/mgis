#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(Auxiliary, isQuotedSql)
{
    int quote = GAIA_SQL_SINGLE_QUOTE;
    std::string actual = Auxiliary::quotedSql("SELECT foo FROM 'bar';", quote);
    std::string expect = "SELECT foo FROM ''bar'';";
    EXPECT_EQ(expect, actual);
}

TEST(Auxiliary, isDoubleQuotedSql)
{
    std::string actual = Auxiliary::doubleQuotedSql("SELECT foo FROM 'bar';");
    std::string expect = "SELECT foo FROM 'bar';";
    EXPECT_EQ(expect, actual);
}

TEST(Auxiliary, isDequotedSql)
{
    std::string actual = Auxiliary::dequotedSql("SELECT foo FROM 'bar';");
    std::string expect = "SELECT foo FROM 'bar';";
    EXPECT_EQ(expect, actual);
}

TEST(Auxiliary, isStringInvalid)
{
    EXPECT_THROW(Auxiliary::toString(0, ""), std::runtime_error);
}
