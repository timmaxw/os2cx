#include <assert.h>
#include <math.h>

#include <sstream>

#include <gtest/gtest.h>

#include "openscad_value.hpp"

namespace os2cx {

TEST(OpenscadValueTest, ParseOne) {
    OpenscadValue v;

    v = OpenscadValue::parse_one("true");
    assert(v == OpenscadValue::make_bool(true));

    v = OpenscadValue::parse_one("false");
    assert(v == OpenscadValue::make_bool(false));

    v = OpenscadValue::parse_one("123");
    assert(v == OpenscadValue(123.0));

    v = OpenscadValue::parse_one("-123");
    assert(v == OpenscadValue(-123.0));

    v = OpenscadValue::parse_one("123.456");
    assert(v.type == OpenscadValue::Type::Number);
    assert(fabs(v.number_value - 123.456) < 1e-10);

    v = OpenscadValue::parse_one("inf");
    assert(v == OpenscadValue(INFINITY));

    v = OpenscadValue::parse_one("[0:1:10]");
    assert(v.type == OpenscadValue::Type::Range);
    assert(v.range_value.start == 0);
    assert(v.range_value.step == 1);
    assert(v.range_value.end == 10);

    v = OpenscadValue::parse_one("\"abc def\"");
    assert(v == OpenscadValue("abc def"));

    v = OpenscadValue::parse_one("undef");
    assert(v == OpenscadValue());

    v = OpenscadValue::parse_one("[1,2,3]");
    OpenscadValue expect(std::vector<OpenscadValue>({
        OpenscadValue(1.0),
        OpenscadValue(2.0),
        OpenscadValue(3.0)
    }));
    assert(v == expect);
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

    assert(str == "[true,123,[0:1:10],\"abc def\",undef]");
}

} /* namespace os2cx */
