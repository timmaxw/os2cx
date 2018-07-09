#include "units.hpp"

#include "math.h"

#include <map>
#include <set>
#include <sstream>

namespace os2cx {

const UnitType UnitType::dimensionless        ( 0,  0,  0,  0,  0,  0,  0);

const UnitType UnitType::length               ( 1,  0,  0,  0,  0,  0,  0);
const UnitType UnitType::mass                 ( 0,  1,  0,  0,  0,  0,  0);
const UnitType UnitType::time                 ( 0,  0,  1,  0,  0,  0,  0);
const UnitType UnitType::temperature          ( 0,  0,  0,  1,  0,  0,  0);
const UnitType UnitType::electric_current     ( 0,  0,  0,  0,  1,  0,  0);
const UnitType UnitType::amount_of_substance  ( 0,  0,  0,  0,  0,  1,  0);
const UnitType UnitType::luminous_intensity   ( 0,  0,  0,  0,  0,  0,  1);

const UnitType UnitType::acceleration         ( 1,  0, -2,  0,  0,  0,  0);
const UnitType UnitType::area                 ( 2,  0,  0,  0,  0,  0,  0);
const UnitType UnitType::capacitance          (-2, -1,  4,  0,  2,  0,  0);
const UnitType UnitType::density              (-3,  1,  0,  0,  0,  0,  0);
const UnitType UnitType::dynamic_viscosity    (-1,  1, -1,  0,  0,  0,  0);
const UnitType UnitType::electric_charge      ( 0,  0,  1,  0,  1,  0,  0);
const UnitType UnitType::electric_conductance (-2, -1,  3,  0,  2,  0,  0);
const UnitType UnitType::electric_potential   ( 2,  1, -3,  0, -1,  0,  0);
const UnitType UnitType::electric_resistance  ( 2,  1, -3,  0, -2,  0,  0);
const UnitType UnitType::energy               ( 2,  1, -2,  0,  0,  0,  0);
const UnitType UnitType::force                ( 1,  1, -2,  0,  0,  0,  0);
const UnitType UnitType::frequency            ( 0,  0, -1,  0,  0,  0,  0);
const UnitType UnitType::inductance           ( 2,  1, -2,  0, -2,  0,  0);
const UnitType UnitType::kinematic_viscosity  ( 2,  0, -1,  0,  0,  0,  0);
const UnitType UnitType::magnetic_flux        ( 2,  1, -2,  0, -1,  0,  0);
const UnitType UnitType::magnetic_flux_density( 0,  1, -2,  0, -1,  0,  0);
const UnitType UnitType::power                ( 2,  1, -3,  0,  0,  0,  0);
const UnitType UnitType::pressure             (-1,  1, -2,  0,  0,  0,  0);
const UnitType UnitType::speed                ( 1,  0, -1,  0,  0,  0,  0);
const UnitType UnitType::volume               ( 3,  0,  0,  0,  0,  0,  0);

UnitType::UnitType(
    int lp, int mp, int tp, int ecp, int tep, int aosp, int lip
) :
    length_power(lp),
    mass_power(mp),
    time_power(tp),
    electric_current_power(ecp),
    temperature_power(tep),
    amount_of_substance_power(aosp),
    luminous_intensity_power(lip)
    { }

bool UnitType::operator==(const UnitType &other) const {
    return length_power == other.length_power
        && mass_power == other.mass_power
        && time_power == other.time_power
        && electric_current_power == other.electric_current_power
        && temperature_power == other.temperature_power
        && amount_of_substance_power == other.amount_of_substance_power
        && luminous_intensity_power == other.luminous_intensity_power;
}

bool UnitType::operator!=(const UnitType &other) const {
    return !(*this == other);
}

const std::map<std::string, int> si_prefix_exponents {
    {"y",  -24}, {"z",  -21}, {"a",  -18}, {"f",  -15},
    {"p",  -12}, {"n",   -9}, {"u",   -6}, {"m",   -3},
    {"c",   -2}, {"d",   -1}, {"da",  +1}, {"h",   +2},
    {"k",   +3}, {"M",   +6}, {"G",   +9}, {"T",  +12},
    {"P",  +15}, {"E",  +18}, {"Z",  +21}, {"Y",  +24}
};

const std::map<std::string, Unit> unit_values {
    /* Fundamental SI units */
    {"m",    Unit(UnitType::length,                1)},
    {"g",    Unit(UnitType::mass,                  1.0e-3)},
    {"s",    Unit(UnitType::time,                  1)},
    {"K",    Unit(UnitType::temperature,           1)},
    {"A",    Unit(UnitType::electric_current,      1)},
    {"mol",  Unit(UnitType::amount_of_substance,   1)},
    {"cd",   Unit(UnitType::luminous_intensity,    1)},

    /* Other SI units */
    {"F",    Unit(UnitType::capacitance,           1)},
    {"C",    Unit(UnitType::electric_charge,       1)},
    {"S",    Unit(UnitType::electric_conductance,  1)},
    {"V",    Unit(UnitType::electric_potential,    1)},
    {"ohm",  Unit(UnitType::electric_resistance,   1)},
    {"J",    Unit(UnitType::energy,                1)},
    {"Hz",   Unit(UnitType::frequency,             1)},
    {"N",    Unit(UnitType::force,                 1)},
    {"H",    Unit(UnitType::inductance,            1)},
    {"Wb",   Unit(UnitType::magnetic_flux,         1)},
    {"T",    Unit(UnitType::magnetic_flux_density, 1)},
    {"Pa",   Unit(UnitType::pressure,              1)},
    {"W",    Unit(UnitType::power,                 1)},
    {"degC", Unit(UnitType::temperature,           1, 273.15)},

    /* Non-fundamental metric units */
    {"min",  Unit(UnitType::time,                  60)},
    {"h",    Unit(UnitType::time,                  60 * 60)},
    {"d",    Unit(UnitType::time,                  24 * 60 * 60)},
    {"L",    Unit(UnitType::volume,                1.0e-3)},

    /* Non-metric units */
    {"lbf",  Unit(UnitType::force,                 4.448222e+0)},
    {"in",   Unit(UnitType::length,                2.54e-2)},
    {"ft",   Unit(UnitType::length,                12 * 2.54e-2)},
    {"lbm",  Unit(UnitType::mass,                  4.5359237e-1)},
    {"degF", Unit(UnitType::temperature,           5 / 9.0, 255.372)}
};

const std::set<std::string> non_prefixable_units {
    "degC", "min", "h", "d", "lbf", "in", "ft", "lbm", "degF"
};

Unit Unit::parse_simple(const std::string &str) {
    auto unit_it = unit_values.find(str);
    if (unit_it != unit_values.end()) {
        return unit_it->second;
    }

    for (const auto &pair : unit_values) {
        if (str.length() < pair.first.length() ||
                str.substr(str.length() - pair.first.length()) != pair.first) {
            continue;
        }
        auto prefix_it = si_prefix_exponents.find(
            str.substr(0, str.length() - pair.first.length()));
        if (prefix_it == si_prefix_exponents.end()) {
            continue;
        }
        if (non_prefixable_units.count(pair.first)) {
            throw UnitParseError("Unit '" + pair.first + "' may not be used "
                "with SI prefixes.");
        }
        Unit unit = pair.second;
        unit.unit_in_si *= pow(10, prefix_it->second);
        return unit;
    }

    if (str == "lb") {
        throw UnitParseError("Please write 'lb' as either 'lbf' or 'lbm' to "
            "explicitly clarify whether you mean pound-force or pound-mass.");
    }

    std::stringstream msg;
    msg << "Unsupported unit '" << str << "'. Supported units are: ";
    for (const auto &pair : unit_values) {
        if (pair.first != unit_values.begin()->first) {
            msg << ", ";
        }
        msg << "'" << pair.first << "'";
    }
    msg << ".";
    throw UnitParseError(msg.str());
}

Unit Unit::parse_compound(const std::string &str) {
    if (str == "dimensionless") {
        return Unit(UnitType::dimensionless, 1);
    }

    static const std::string space_msg = "Unexpected extra space(s) in unit "
        "string. There should be exactly one space between each two units, "
        "and no other other spaces anywhere.";
    static const std::string dimensionless_msg = " (If you want to specify "
        "that a value is dimensionless, pass the literal string "
        "'dimensionless' instead of passing any units.)";

    if (str == "") {
        throw UnitParseError(
            "Expected a unit, got an empty string." + dimensionless_msg);
    }

    std::map<std::string, int> units;
    const char *ptr = str.c_str();
    while (true) {
        if (*ptr == ' ') throw UnitParseError(space_msg);

        const char *unit_end = ptr;
        while (isalpha(*unit_end)) ++unit_end;
        if (unit_end == ptr) {
            throw UnitParseError("Expected a unit abbreviation, got '" +
                std::string(ptr) + "'");
        }
        std::string unit_str(ptr, unit_end);
        ptr = unit_end;

        int power;
        if (*ptr == '^') {
            ++ptr;
            const char *power_end = ptr;
            if (*power_end == '-') ++power_end;
            while (*power_end >= '0' && *power_end <= '9') ++power_end;
            try {
                size_t end_offset;
                power = std::stoi(std::string(ptr, power_end), &end_offset);
                if (ptr + end_offset != power_end) {
                    /* this will be caught below */
                    throw std::invalid_argument("");
                }
                if (power > 100 || power < -100) {
                    /* this will be caught below */
                    throw std::out_of_range("");
                }
            } catch (std::invalid_argument) {
                throw UnitParseError("Expected a number after '^', got '" +
                    std::string(ptr) + "'");
            } catch (std::out_of_range) {
                throw UnitParseError("Units may not be raised to excessively "
                    "large or small powers.");
            }
            if (power == 0) {
                throw UnitParseError("Units may not be raised to the zero-th "
                    "power." + dimensionless_msg);
            } else if (power == 1) {
                throw UnitParseError("Units may not be raised to the one-th "
                    "power. Please just specify the unit with no exponent.");
            }
            ptr = power_end;
        } else {
            power = 1;
        }

        auto res = units.insert(std::make_pair(unit_str, power));
        if (!res.second) {
            throw UnitParseError("The same unit should not be specified "
                "twice. (If you want to square or cube a unit, use the '^' "
                "operator, as in 'm^2'.)");
        }

        bool has_space = (*ptr == ' ');
        if (has_space) ++ptr;

        if (*ptr == '\0') {
            if (has_space) throw UnitParseError(space_msg);
            break;
        } else if (isalpha(*ptr) && !has_space) {
            throw UnitParseError("Units must be separated by spaces.");
        } else if (*ptr == '*') {
            throw UnitParseError("To multiply units, separate them by spaces "
                "instead of using '*'.");
        } else if (*ptr == '/') {
            throw UnitParseError("To invert units, raise them to a negative "
                "power instead of using '/', as in 'km h^-1'.");
        }
    }

    /* This shortcut lets 'degC' and 'degF' bypass the check in the loop */
    if (units.size() == 1 && units.begin()->second == 1) {
        return parse_simple(units.begin()->first);
    }

    Unit combined_unit(UnitType::dimensionless, 1);
    for (const auto &pair : units) {
        Unit unit = parse_simple(pair.first);
        if (unit.zero_in_si != 0) {
            throw UnitParseError("Unit '" + pair.first + "' cannot be "
                "combined with other units or raised to a power, because the "
                "zero-point is arbitrary so this wouldn't make sense.");
        }
        combined_unit.type.length_power += unit.type.length_power * pair.second;
        combined_unit.type.mass_power += unit.type.mass_power * pair.second;
        combined_unit.type.time_power += unit.type.time_power * pair.second;
        combined_unit.type.temperature_power +=
            unit.type.temperature_power * pair.second;
        combined_unit.type.electric_current_power +=
            unit.type.electric_current_power * pair.second;
        combined_unit.type.amount_of_substance_power +=
            unit.type.amount_of_substance_power * pair.second;
        combined_unit.type.luminous_intensity_power +=
            unit.type.luminous_intensity_power * pair.second;
        combined_unit.unit_in_si *= pow(unit.unit_in_si, pair.second);
    }

    return combined_unit;
}

} /* namespace os2cx */
