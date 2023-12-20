#include "gtest/gtest.h"
#include "SpatiaLiteCpp/SpatiaLiteCpp.h"

using namespace SpatiaLite;

TEST(Converter, isValid)
{
    EXPECT_NO_THROW(ConverterPtr(new Converter("ASCII")));
}

TEST(Converter, isConvertValid)
{
    ConverterPtr converter(new Converter("ASCII"));
    const char x = '0';
    EXPECT_NO_THROW(converter->convert(&x, 1));
}
