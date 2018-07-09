#include "units.hpp"

namespace os2cx {

const UnitType UnitType::length            ( 1,  0,  0,  0,  0,  0,  0);
const UnitType UnitType::mass              ( 0,  1,  0,  0,  0,  0,  0);
const UnitType UnitType::time              ( 0,  0,  1,  0,  0,  0,  0);
const UnitType UnitType::temperature       ( 0,  0,  0,  1,  0,  0,  0);
const UnitType UnitType::electric_current  ( 0,  0,  0,  0,  1,  0,  0);
const UnitType UnitType::chemical_amount   ( 0,  0,  0,  0,  0,  1,  0);
const UnitType UnitType::luminous_intensity( 0,  0,  0,  0,  0,  0,  1);

const UnitType UnitType::area              ( 2,  0,  0,  0,  0,  0,  0);
const UnitType UnitType::volume            ( 3,  0,  0,  0,  0,  0,  0);
const UnitType UnitType::force             ( 1,  1, -2,  0,  0,  0,  0);
const UnitType UnitType::pressure          (-1,  1, -2,  0,  0,  0,  0);

const std::map<std::string, int> si_prefix_exponents {
    {"y",  -24}, {"z",  -21}, {"a",  -18}, {"f",  -15},
    {"p",  -12}, {"n",   -9}, {"u",   -6}, {"m",   -3},
    {"c",   -2}, {"d",   -1},
    {"",     0}, /* no prefix (for completeness) */
    {"da",  +1}, {"h",   +2},
    {"k",   +3}, {"M",   +6}, {"G",   +9}, {"T",  +12},
    {"P",  +15}, {"E",  +18}, {"Z",  +21}, {"Y",  +24}
};

const std::map<std::string, std::string> si_prefix_misspellings {
    {"dk",       "da"}, /* historical abbreviation for 'deka-' */
    {u8"\u00B5", "u" }, /* Unicode 'micro' sign */
    {u8"\u03BC", "u" }  /* Unicode Greek letter 'mu' */
};

const std::map<std::string, std::string> si_prefix_long_names {
    {"yocto", "y" }, {"zepto", "z" }, {"atto",  "a" }, {"femto", "f" },
    {"pico",  "p" }, {"nano",  "n" }, {"micro", "u" }, {"milli", "m" },
    {"centi", "c" }, {"deci",  "d" },
    {"",      ""  }, /* no prefix (for completeness) */
    {"deca",  "da"}, {"hecto", "h" },
    {"kilo",  "k" }, {"mega",  "M" }, {"giga",  "G" }, {"tera",  "T" },
    {"peta",  "P" }, {"exa",   "E" }, {"zetta", "Z" }, {"yotta", "Y" },
    {"deka",  "da"}, /* alternate spelling of 'deca-' */
};

double parse_si_prefix()

double parse_si_prefix(
        std::string prefix_str,
        const std::string &unit_str,
        UnitParseError *warning_out
) {
    if (si_prefix_misspellings.count(prefix)) {
        std::string preferred = si_prefix_misspellings.at(prefix);
        *warning_out = UnitParseError("Please spell the SI prefix '" + prefix +
            "-' as '" + preferred + "-'.")
        prefix = preferred;
    } else if (!si_prefix_exponents.count(prefix)) {
        throw UnitParseError("'" + prefix + unit + " is not a valid unit "
            "because '" + prefix + "-' is not a valid SI prefix. Try a "
            "supported unit like '" + unit + "', 'm" + unit + "', or 'k" +
            unit + "' instead.");
    }
    int exponent = si_prefix_exponents.at(prefix);

    /* Ban non-power-of-1000 prefixes (except for the special case of "cm")
    because they're rarely used, and I'd rather not support weird corner cases
    that probably nobody will ever want */
    if ((exponent == -1 || exponent == -2) &&
            !(prefix_str == "c" && unit_str == "m")) {
        *warning_out = UnitParseError("Please don't use SI prefix '" + prefix +
            "-', because it's uncommon. Instead, please work in units of '" +
            unit_str + "' or 'm" + unit_str + "'.");
    }
    if (exponent == 1 || exponent == 2) {
        *warning_out = UnitParseError("Please don't use SI prefix '" + prefix +
            "-' because it's uncommon. Instead, please work in units of '" +
            unit_str + "' or 'k" + unit_str + "'.");
    }

    return power(10, exponent);
}

const std::map<std::string, Unit> unit_values_allowed {
    /* Fundamental metric units */
    {"m",    Unit(UnitType::length,             1)},
    {"g",    Unit(UnitType::mass,               1.0e-3)},
    {"s",    Unit(UnitType::time,               1)},
    {"K",    Unit(UnitType::temperature,        1)},
    {"A",    Unit(UnitType::electric_current,   1)},
    {"mol",  Unit(UnitType::chemical_amount,    1)},
    {"cd",   Unit(UnitType::luminous_intensity, 1)},

    /* Non-fundamental metric units */
    {"L",    Unit(UnitType::volume,           1.0e-3)},
    {"min",  Unit(UnitType::time,             60)},
    {"h",    Unit(UnitType::time,             60 * 60)},
    {"d",    Unit(UnitType::time,             24 * 60 * 60)},
    {"N",    Unit(UnitType::force,            1)},
    {"Pa",   Unit(UnitType::pressure,         1)},
    {"degC", Unit(UnitType::temperature,      1, 273.15)},

    /* Non-metric units */
    {"in",   Unit(UnitType::length,           2.54e-2)},
    {"ft",   Unit(UnitType::length,           12 * 2.54e-2)},
    {"lbm",  Unit(UnitType::mass,             4.5359237e-1)},
    {"lbf",  Unit(UnitType::force,            4.448222e+0)},
    {"degF", Unit(UnitType::temperature,      5 / 9.0, 255.372)}
};

const std::map<std::string, UnitType> unit_bans {
    {"a",    UnitType::time},
    {"degR", UnitType::temperature},
};

const std::map<std::string, std::string> unit_misspellings {
    /* Alternative abbreviations that are in common use, but not standard */
    {"sec",       "s"   },
    {"yr",        "a"   },
    {"hr",        "h"   },

    /* Kelvin shouldn't have the word "degrees", but Rankine should */
    {"degK",      "K"   },
    {"R",         "degR"},

    /* Don't allow the Unicode 'degree' symbol */
    {u8"\u00B0K", "K"   },
    {u8"\u00B0C", "degC"},
    {u8"\u2103",  "degC"},
    {u8"\u00B0R", "degR"},
    {u8"\u00B0F", "degF"},

    /* GNU Units uses 'tempC' and 'tempF' */
    {"tempC",     "degC"},
    {"tempF",     "degF"},

    /* The Unified Code for Units of Measure uses 'Cel' */
    {"Cel",       "degC"},


    {""}

const std::map<std::string, std::string> unit_long_names {
    {"meter", "m"}, {"metre", "m"},
    {"gram", "g"}, {"gramme", "g"},

    {"inch", "in"}, {""}
};

const std::set<std::string> prefixable_unit_set {
    "m", "g", "s", "K", "A", "mol", "cd", "L", "N", "Pa"
};

Unit Unit::parse(const std::string &str) {
    auto unit_it = unit_map.find(str);
    if (unit_it != unit_map.end()) {
        return unit_it->second;
    }

    for (const std::string &base_str : prefixable_unit_set) {
        if (str.length() < base_str.length() ||
                str.substr(str.length() - base_str.length()) != base_str) {
            continue;
        }
        auto prefix_it = prefix_map.find(
            str.substr(0, str.length() - base_str.length()));
        if (prefix_it == prefix_map.end()) continue;

        /* Ban extreme prefixes ('y-', 'z-', 'a-', 'f-', 'P-', 'E-', 'Z-', 'Y-')
        and prefixes that aren't a power of 1000 ('c-', 'd-', 'da-', 'h-')
        because they aren't in common use, so they may be confusing to people
        reading the OpenSCAD file. However, we allow 'cm' as a special case,
        because it's so common. */
        if (prefix_it->second > 15 || prefix_it->second < -15) {
            throw UnitParseError("SI prefix '" + prefix_it->first + "-' is not "
                "allowed. In general, extremely large or small metric prefixes "
                "('y-', 'z-', 'a-', 'E-', 'Z-', and 'Y-') are not allowed "
                "because they are uncommon and people reading the code may not "
                "be familiar with them.");
        }
        if (prefix_it->second % 3 != 0 && str != "cm") {
            throw UnitParseError("SI prefix '" + prefix_it->first + "-' is not "
                "allowed. "
        }

        Unit base_unit =
    }
}

} /* namespace os2cx */
