#ifndef OS2CX_OPENSCAD_VALUE_HPP_
#define OS2CX_OPENSCAD_VALUE_HPP_

#include <stdexcept>
#include <string>
#include <vector>

namespace os2cx {

class OpenscadValue {
public:
    enum class Type {
        Bool,
        Number,
        Range,
        String,
        Undefined,
        Vector
    };

    class ParseError : public std::runtime_error {
    public:
        ParseError(const std::string &msg) : std::runtime_error(msg) { }
    };

    class Range {
    public:
        double start, step, end;
    };

    /* Note that '\0' and '\n' are both considered to indicate the end of the
    string. */
    static OpenscadValue parse_one(const char *string);
    static std::vector<OpenscadValue> parse_many(const char *string);

    OpenscadValue() : type(Type::Undefined) { }
    static OpenscadValue make_bool(bool v) {
        /* Many types implicitly cast to bool, so this has to be a static
        method rather than a constructor */
        OpenscadValue value;
        value.type = Type::Bool;
        value.bool_value = v;
        return value;
    }
    explicit OpenscadValue(double v)
        : type(Type::Number), number_value(v) { }
    explicit OpenscadValue(const Range &v)
        : type(Type::Range), range_value(v) { }
    explicit OpenscadValue(const std::string &v)
        : type(Type::String), string_value(v) { }
    explicit OpenscadValue(std::vector<OpenscadValue> &&v)
        : type(Type::Vector), vector_value(v) { }

    Type type;
    bool bool_value;
    double number_value;
    Range range_value;
    std::string string_value;
    std::vector<OpenscadValue> vector_value;
};

std::ostream &operator<<(std::ostream &stream, const OpenscadValue &value);
bool operator==(const OpenscadValue &x, const OpenscadValue &y);
inline bool operator!=(const OpenscadValue &x, const OpenscadValue &y) {
    return !(x == y);
}

} /*namespace os2cx */

#endif
