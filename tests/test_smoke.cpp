#include <gtest/gtest.h>
#include "jsonlib/value.hpp"

using namespace jsonlib;

TEST(Smoke, ConstructBasics)
{
    value vnull;
    value vtrue{true};
    value vfalse{false};
    value vnumber{42.0};
    value vstring{"Hello, World!"};
    array a{vtrue, vnumber, vstring};
    object o{{"key1", vfalse}, {"key2", value{"value2"}}};

    EXPECT_TRUE(vnull.is_null());
    EXPECT_TRUE(vtrue.is_bool());
    EXPECT_TRUE(vfalse.is_bool());
    EXPECT_TRUE(vnumber.is_number());
    EXPECT_TRUE(vstring.is_string());
    EXPECT_TRUE(value{a}.is_array());
    EXPECT_TRUE(value{o}.is_object());
}