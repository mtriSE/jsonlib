#include <gtest/gtest.h>

#include "jsonlib/parse.hpp"

using namespace jsonlib;

TEST(Parse, BasicValue_Test) {
    // Null
    EXPECT_TRUE(parse("null").is_null());

    // Boolean
    EXPECT_TRUE(parse("true").is_bool());
    EXPECT_TRUE(parse("true").as_bool());
    EXPECT_TRUE(parse("false").is_bool());
    EXPECT_FALSE(parse("false").as_bool());

    // Number
    auto num = parse("42.5");
    EXPECT_TRUE(num.is_number());
    EXPECT_DOUBLE_EQ(num.as_number(), 42.5);
}

TEST(Parse, BasicString_Test) {
    // String
    auto str = parse(R"("Hello, World!")");
    EXPECT_TRUE(str.is_string());
    EXPECT_EQ(str.as_string(), "Hello, World!");
}

TEST(Parse, BasicArray_Test) {
    // Array
    auto arr = parse(R"([true, 42, "text"])");
    EXPECT_TRUE(arr.is_array());
    EXPECT_EQ(arr.as_array().size(), 3);
    for (const auto& v : arr.as_array()) {
        EXPECT_TRUE(v.is_bool() || v.is_number() || v.is_string());
    }
}
