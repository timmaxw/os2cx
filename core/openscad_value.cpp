#include "openscad_value.hpp"

#include <assert.h>
#include <math.h>

#include <iostream>

namespace os2cx {

class OpenscadValueParser {
public:
    OpenscadValueParser(const char *i) :
        original_input(i), input(i) { }

    void fail(const std::string &message) {
        throw OpenscadValue::ParseError(
            "Failed to parse OpenSCAD value: " + message
            + "\nError is near \"" + std::string(input).substr(0, 10) + "\""
            + "\nInput is:\n" + original_input);
    }

    void skip_whitespace() {
        while (*input == ' ') {
            ++input;
        }
    }

    bool is_end() {
        return *input == '\0' || *input == '\n';
    }

    /* All parsing functions expect there to be no leading whitespace before the
    entity they are parsing. All parsing functions must consume any trailing
    whitespace after the entity they parse.

    If a function with a name of the form parse_foo() encounters invalid syntax,
    it calls fail() and leaves the input pointer in an undefined state.

    Functions with names of the form try_parse_foo() attempt to check whether
    the input was even meant to be a "foo" or not. If it clearly isn't a "foo",
    then try_parse_foo() puts the input pointer back at the start of the object
    and returns false. If it looks like a foo but malformed, try_parse_foo()
    calls fail() just like parse_foo() does. */

    double parse_double() {
        char *end;
        double value = strtod(input, &end);
        if (end == input) {
            fail("invalid number");
        }
        input = end;
        skip_whitespace();
        return value;
    }

    bool try_parse_word(const char *str) {
        char const *backtrack = input;
        while (*str != '\0') {
            if (*str == *input) {
                ++str;
                ++input;
            } else {
                input = backtrack;
                return false;
            }
        }
        skip_whitespace();
        return true;
    }

    /* Only parses the part inside the brackets, not the brackets themselves. */
    bool try_parse_range(OpenscadValue::Range *range_out) {
        char const *backtrack = input;
        try {
            range_out->start = parse_double();
        } catch (OpenscadValue::ParseError) {
            input = backtrack;
            return false;
        }
        if (!try_parse_word(":")) {
            input = backtrack;
            return false;
        }
        range_out->step = parse_double();
        if (try_parse_word(":")) {
            range_out->end = parse_double();
        } else {
            range_out->end = range_out->step;
            range_out->step = 1.0;
        }
        return true;
    }

    OpenscadValue parse_openscad_value() {
        OpenscadValue value;
        if (try_parse_word("true")) {
            value.type = OpenscadValue::Type::Bool;
            value.bool_value = true;
        } else if (try_parse_word("false")) {
            value.type = OpenscadValue::Type::Bool;
            value.bool_value = false;
        } else if (try_parse_word("inf")) {
            value.type = OpenscadValue::Type::Number;
            value.number_value = INFINITY;
        } else if (try_parse_word("-inf")) {
            value.type = OpenscadValue::Type::Number;
            value.number_value = -INFINITY;
        } else if (try_parse_word("nan")) {
            value.type = OpenscadValue::Type::Number;
            value.number_value = NAN;
        } else if (isdigit(*input) || *input == '-') {
            value.type = OpenscadValue::Type::Number;
            value.number_value = parse_double();
        } else if (try_parse_word("[")) {
            if (try_parse_range(&value.range_value)) {
                value.type = OpenscadValue::Type::Range;
                if (!try_parse_word("]")) {
                    fail("expected ']' at end of range");
                }
            } else {
                value.type = OpenscadValue::Type::Vector;
                if (*input != ']') {
                    while (true) {
                        value.vector_value.push_back(parse_openscad_value());
                        if (!try_parse_word(",")) {
                            break;
                        }
                    }
                }
                if (!try_parse_word("]")) {
                    fail("expected ',' or ']'");
                }
            }
        } else if (*input == '"') {
            ++input;
            value.type = OpenscadValue::Type::String;
            while (*input != '"') {
                if (is_end()) {
                    fail("unterminated string");
                }
                value.string_value += *input;
                ++input;
            }
            ++input;
            skip_whitespace();
        } else if (try_parse_word("undef")) {
            value.type = OpenscadValue::Type::Undefined;
        } else {
            fail("unrecognizable nonsense");
        }
        return value;
    }

    const char *original_input;
    const char *input;
};

OpenscadValue OpenscadValue::parse_one(const char *string) {
    OpenscadValueParser parser(string);
    parser.skip_whitespace();
    std::vector<OpenscadValue> values;
    OpenscadValue value = parser.parse_openscad_value();
    if (!parser.is_end()) {
        parser.fail("found something after value in string");
    }
    return value;
}

std::vector<OpenscadValue> OpenscadValue::parse_many(const char *string) {
    OpenscadValueParser parser(string);
    parser.skip_whitespace();
    std::vector<OpenscadValue> values;
    if (parser.is_end()) {
        return values;
    }
    while (true) {
        values.push_back(parser.parse_openscad_value());
        if (!parser.try_parse_word(",")) {
            break;
        }
    }
    if (!parser.is_end()) {
        parser.fail("expected ',' or end of input");
    }
    return values;
}

std::ostream &operator<<(std::ostream &stream, const OpenscadValue &v) {
    switch (v.type) {
    case OpenscadValue::Type::Bool:
        stream << (v.bool_value ? "true" : "false");
        break;
    case OpenscadValue::Type::Number:
        stream << v.number_value;
        break;
    case OpenscadValue::Type::Range:
        stream << "[" << v.range_value.start
            << ":" << v.range_value.step
            << ":" << v.range_value.end << "]";
        break;
    case OpenscadValue::Type::String:
        stream << '"' << v.string_value << '"';
        break;
    case OpenscadValue::Type::Undefined:
        stream << "undef";
        break;
    case OpenscadValue::Type::Vector:
        stream << "[";
        for (size_t i = 0; i < v.vector_value.size(); ++i) {
            if (i != 0) stream << ",";
            stream << v.vector_value[i];
        }
        stream << "]";
        break;
    default: assert(false);
    }
    return stream;
}

bool operator==(const OpenscadValue &x, const OpenscadValue &y) {
    if (x.type != y.type) return false;
    switch (x.type) {
    case OpenscadValue::Type::Bool:
        return x.bool_value == y.bool_value;
    case OpenscadValue::Type::Number:
        return x.number_value == y.number_value;
    case OpenscadValue::Type::Range:
        return x.range_value.start == y.range_value.start
            && x.range_value.step == y.range_value.step
            && x.range_value.end == y.range_value.end;
    case OpenscadValue::Type::String:
        return x.string_value == y.string_value;
    case OpenscadValue::Type::Undefined:
        return true;
    case OpenscadValue::Type::Vector:
        if (x.vector_value.size() != y.vector_value.size()) return false;
        for (size_t i = 0; i < x.vector_value.size(); ++i) {
            if (x.vector_value[i] != y.vector_value[i]) return false;
        }
        return true;
    default: assert(false);
    }
}

} /* namespace os2cx */

