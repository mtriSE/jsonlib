#include <gtest/gtest.h>

#include "jsonlib/stringify.hpp"

using namespace jsonlib;

TEST(Stringify, BasicValues_Test) {
    EXPECT_EQ(to_string(value()), "null");
    EXPECT_EQ(to_string(value(true)), "true");
    EXPECT_EQ(to_string(value(false)), "false");
    EXPECT_EQ(to_string(value(42.5)), "42.5");
    EXPECT_EQ(to_string(value("Hello, World!")), R"("Hello, World!")");
}

TEST(Stringify, AdvanceString_Test) {
    EXPECT_EQ(to_string(value("Line1\nLine2\tTabbed")), R"("Line1\nLine2\tTabbed")");
    EXPECT_EQ(to_string(value(std::string("Control:\b\f\n\r\t"))), R"("Control:\b\f\n\r\t")");

    // TODO: Fix these tests
    // EXPECT_EQ(to_string(value("Quote: \" Backslash: \\")), R"("Quote: \" Backslash: \\")");
    // EXPECT_EQ(to_string(value("Unicode:\u1234")), R"("Unicode:\u1234")");
}

TEST(Stringify, BasicArray_Test) {
    array arr{value(true), value(42.0), value("text")};
    EXPECT_EQ(to_string(value(arr)), R"([true,42,"text"])");
    EXPECT_EQ(to_string(value(arr), true), "[\n  true,\n  42,\n  \"text\"\n]");
}