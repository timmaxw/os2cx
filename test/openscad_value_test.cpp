#include <math.h>

#include <sstream>

#include <gtest/gtest.h>

#include "openscad_value.hpp"

namespace os2cx {

TEST(OpenscadValueTest, ParseOne) {
    OpenscadValue v;

    v = OpenscadValue::parse_one("true");
    EXPECT_EQ(OpenscadValue::make_bool(true), v);

    v = OpenscadValue::parse_one("false");
    EXPECT_EQ(OpenscadValue::make_bool(false), v);

    v = OpenscadValue::parse_one("123");
    EXPECT_EQ(OpenscadValue(123.0), v);

    v = OpenscadValue::parse_one("-123");
    EXPECT_EQ(OpenscadValue(-123.0), v);

    v = OpenscadValue::parse_one("123.456");
    EXPECT_EQ(OpenscadValue::Type::Number, v.type);
    EXPECT_FLOAT_EQ(123.456, v.number_value);

    v = OpenscadValue::parse_one("inf");
    EXPECT_EQ(OpenscadValue(INFINITY), v);

    v = OpenscadValue::parse_one("[0:1:10]");
    OpenscadValue expected_range(OpenscadValue::Range { 0, 1, 10 });
    EXPECT_EQ(expected_range, v);

    v = OpenscadValue::parse_one("\"abc def\"");
    EXPECT_EQ(OpenscadValue("abc def"), v);

    v = OpenscadValue::parse_one("undef");
    EXPECT_EQ(OpenscadValue(), v);

    v = OpenscadValue::parse_one("[1,2,3]");
    OpenscadValue expected_array(std::vector<OpenscadValue>({
        OpenscadValue(1.0),
        OpenscadValue(2.0),
        OpenscadValue(3.0)
    }));
    EXPECT_EQ(expected_array, v);
}

TEST(OpenscadValueTest, ToString) {
    OpenscadValue v(std::vector<OpenscadValue>({
        OpenscadValue::make_bool(true),
        OpenscadValue(123.0),
        OpenscadValue(OpenscadValue::Range {0, 1, 10}),
        OpenscadValue("abc def"),
        OpenscadValue()
    }));

    std::stringstream stream;
    stream << v;
    std::string str = stream.str();

    EXPECT_EQ("[true,123,[0:1:10],\"abc def\",undef]", str);
}

} /* namespace os2cx */
