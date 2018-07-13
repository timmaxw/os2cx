#include "units.hpp"

#include "math.h"

#include <map>
#include <set>
#include <sstream>

namespace os2cx {

UnitType UnitType::dimensionless() {
    return UnitType( 0,  0,  0,  0,  0,  0,  0); }

UnitType UnitType::length() {
    return UnitType( 1,  0,  0,  0,  0,  0,  0); }
UnitType UnitType::mass() {
    return UnitType( 0,  1,  0,  0,  0,  0,  0); }
UnitType UnitType::time() {
    return UnitType( 0,  0,  1,  0,  0,  0,  0); }
UnitType UnitType::electric_current() {
    return UnitType( 0,  0,  0,  0,  1,  0,  0); }
UnitType UnitType::thermodynamic_temperature() {
    return UnitType( 0,  0,  0,  1,  0,  0,  0); }
UnitType UnitType::amount_of_substance() {
    return UnitType( 0,  0,  0,  0,  0,  1,  0); }
UnitType UnitType::luminous_intensity() {
    return UnitType( 0,  0,  0,  0,  0,  0,  1); }

UnitType UnitType::acceleration() {
    return UnitType( 1,  0, -2,  0,  0,  0,  0); }
UnitType UnitType::area() {
    return UnitType( 2,  0,  0,  0,  0,  0,  0); }
UnitType UnitType::capacitance() {
    return UnitType(-2, -1,  4,  0,  2,  0,  0); }
UnitType UnitType::density() {
    return UnitType(-3,  1,  0,  0,  0,  0,  0); }
UnitType UnitType::dynamic_viscosity() {
    return UnitType(-1,  1, -1,  0,  0,  0,  0); }
UnitType UnitType::electric_charge() {
    return UnitType( 0,  0,  1,  0,  1,  0,  0); }
UnitType UnitType::electric_conductance() {
    return UnitType(-2, -1,  3,  0,  2,  0,  0); }
UnitType UnitType::electric_potential() {
    return UnitType( 2,  1, -3,  0, -1,  0,  0); }
UnitType UnitType::electric_resistance() {
    return UnitType( 2,  1, -3,  0, -2,  0,  0); }
UnitType UnitType::energy() {
    return UnitType( 2,  1, -2,  0,  0,  0,  0); }
UnitType UnitType::force() {
    return UnitType( 1,  1, -2,  0,  0,  0,  0); }
UnitType UnitType::frequency() {
    return UnitType( 0,  0, -1,  0,  0,  0,  0); }
UnitType UnitType::inductance() {
    return UnitType( 2,  1, -2,  0, -2,  0,  0); }
UnitType UnitType::kinematic_viscosity() {
    return UnitType( 2,  0, -1,  0,  0,  0,  0); }
UnitType UnitType::magnetic_flux() {
    return UnitType( 2,  1, -2,  0, -1,  0,  0); }
UnitType UnitType::magnetic_flux_density() {
    return UnitType( 0,  1, -2,  0, -1,  0,  0); }
UnitType UnitType::power() {
    return UnitType( 2,  1, -3,  0,  0,  0,  0); }
UnitType UnitType::pressure() {
    return UnitType(-1,  1, -2,  0,  0,  0,  0); }
UnitType UnitType::speed() {
    return UnitType( 1,  0, -1,  0,  0,  0,  0); }
UnitType UnitType::volume() {
    return UnitType( 3,  0,  0,  0,  0,  0,  0); }

UnitType UnitType::celsius_fahrenheit_temperature() {
    return UnitType( 0,  0,  0,  1,  0,  0,  0, true); }

UnitType::UnitType(
    int lp, int mp, int tp, int ecp, int ttp, int aosp, int lip,
    bool az
) :
    length_power(lp),
    mass_power(mp),
    time_power(tp),
    electric_current_power(ecp),
    thermodynamic_temperature_power(ttp),
    amount_of_substance_power(aosp),
    luminous_intensity_power(lip),
    arbitrary_zero(az)
    { }

bool UnitType::operator==(const UnitType &other) const {
    return length_power == other.length_power
        && mass_power == other.mass_power
        && time_power == other.time_power
        && electric_current_power == other.electric_current_power
        && thermodynamic_temperature_power ==
            other.thermodynamic_temperature_power
        && amount_of_substance_power == other.amount_of_substance_power
        && luminous_intensity_power == other.luminous_intensity_power
        && arbitrary_zero == other.arbitrary_zero;
}

bool UnitType::operator!=(const UnitType &other) const {
    return !(*this == other);
}

class UnitTable {
public:
    UnitTable(const std::vector<Unit> &au) : all_units(au) {
        for (const Unit &unit : all_units) {
            units_by_abbrev.insert(std::make_pair(unit.abbrev, unit));
        }
    }
    std::vector<Unit> all_units;
    std::map<std::string, Unit> units_by_abbrev;
};

const UnitTable &unit_table() {
    static UnitTable ut({
        Unit("m",    UnitType::length,                    1),
        Unit("g",    UnitType::mass,                      1.0e-3),
        Unit("s",    UnitType::time,                      1),
        Unit("A",    UnitType::electric_current,          1),
        Unit("K",    UnitType::thermodynamic_temperature, 1),
        Unit("mol",  UnitType::amount_of_substance,       1),
        Unit("cd",   UnitType::luminous_intensity,        1),

        Unit("C",    UnitType::electric_charge,           1),
        Unit("F",    UnitType::capacitance,               1),
        Unit("H",    UnitType::inductance,                1),
        Unit("Hz",   UnitType::frequency,                 1),
        Unit("J",    UnitType::energy,                    1),
        Unit("N",    UnitType::force,                     1),
        Unit("ohm",  UnitType::electric_resistance,       1),
        Unit("Pa",   UnitType::pressure,                  1),
        Unit("S",    UnitType::electric_conductance,      1),
        Unit("T",    UnitType::magnetic_flux_density,     1),
        Unit("V",    UnitType::electric_potential,        1),
        Unit("W",    UnitType::power,                     1),
        Unit("Wb",   UnitType::magnetic_flux,             1),

        Unit("degC", UnitType::celsius_fahrenheit_temperature,
            1, 273.15),

        Unit("min",  UnitType::time,                      60),
        Unit("h",    UnitType::time,                      60 * 60),
        Unit("d",    UnitType::time,                      24 * 60 * 60),
        Unit("L",    UnitType::volume,                    1.0e-3),

        Unit("ft",   UnitType::length,                    12 * 2.54e-2),
        Unit("in",   UnitType::length,                    2.54e-2),
        Unit("lbf",  UnitType::force,                     4.448222e+0),
        Unit("lbm",  UnitType::mass,                      4.5359237e-1),

        Unit("degF", UnitType::celsius_fahrenheit_temperature,
            5 / 9.0, 255.372)
    });
    return ut;
}

const std::set<std::string> prefixable_units {
    "m", "g", "s", "A", "K", "mol", "cd",
    "C", "F", "H", "Hz", "J", "N", "ohm", "Pa", "S", "T", "V", "W", "Wb",
    "L"
};

const std::set<std::string> imperial_units {
    "ft", "in", "lbf", "lbm", "degF"
};

const std::map<std::string, int> &si_prefix_exponents() {
    static const std::map<std::string, int> spe {
        {"y",  -24}, {"z",  -21}, {"a",  -18}, {"f",  -15},
        {"p",  -12}, {"n",   -9}, {"u",   -6}, {"m",   -3},
        {"c",   -2}, {"d",   -1}, {"da",  +1}, {"h",   +2},
        {"k",   +3}, {"M",   +6}, {"G",   +9}, {"T",  +12},
        {"P",  +15}, {"E",  +18}, {"Z",  +21}, {"Y",  +24}
    };
    return spe;
}

Unit Unit::parse_simple(const std::string &str) {
    auto unit_it = unit_table().units_by_abbrev.find(str);
    if (unit_it != unit_table().units_by_abbrev.end()) {
        return unit_it->second;
    }

    for (const auto &prefix_pair : si_prefix_exponents) {
        if (prefix_pair.first != str.substr(0, prefix_pair.first.length())) {
            continue;
        }
        std::string unit_str = str.substr(prefix_pair.first.length());
        unit_it = unit_table().units_by_abbrev.find(unit_str);
        if (unit_it == unit_table().units_by_abbrev.end()) {
            continue;
        }
        if (!prefixable_units.count(unit_str)) {
            throw UnitParseError("Unit '" + unit_str + "' may not be used with "
                "SI prefixes.");
        }
        return Unit(
            prefix_pair.first + unit_it->second.abbrev,
            unit_it->second.type,
            unit_it->second.unit_in_si * pow(10, prefix_pair.second));
    }

    if (str == "lb") {
        throw UnitParseError("Please write 'lb' as either 'lbf' or 'lbm' to "
            "explicitly clarify whether you mean pound-force or pound-mass.");
    }

    if (str.find(' ') != std::string::npos ||
            str.find('^') != std::string::npos) {
        throw UnitParseError("Expected a single unit, not a compound unit.");
    }

    std::stringstream msg;
    msg << "Unsupported unit '" << str << "'. Supported units are: ";
    for (const Unit &unit : unit_table().all_units) {
        if (unit.abbrev != unit_table().all_units[0].abbrev) {
            msg << ", ";
        }
        msg << "'" << unit.abbrev << "'";
    }
    msg << ".";
    throw UnitParseError(msg.str());
}

Unit Unit::parse_compound(const std::string &str) {
    if (str == "dimensionless") {
        return Unit("dimensionless", UnitType::dimensionless, 1);
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

    Unit combined_unit(str, UnitType::dimensionless, 1);
    for (const auto &pair : units) {
        Unit unit = parse_simple(pair.first);
        if (unit.zero_in_si != 0) {
            throw UnitParseError("Unit '" + pair.first + "' cannot be "
                "combined with other units or raised to a power, because the "
                "zero-point is relative to a reference, so this wouldn't make "
                "sense.");
        }
        combined_unit.type.length_power +=
            unit.type.length_power * pair.second;
        combined_unit.type.mass_power +=
            unit.type.mass_power * pair.second;
        combined_unit.type.time_power +=
            unit.type.time_power * pair.second;
        combined_unit.type.thermodynamic_temperature_power +=
            unit.type.thermodynamic_temperature_power * pair.second;
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

UnitSystem UnitSystem::parse(
    const std::string &length_str,
    const std::string &mass_or_force_str,
    const std::string &time_str,
    const std::string &temperature_str
) {
    UnitSystem system;

    Unit length_unit = Unit::parse_simple(length_str);
    Unit mass_or_force_unit = Unit::parse_simple(mass_str);
    Unit time_unit = Unit::parse_simple(time_str);
    Unit temperature_unit = Unit::parse_simple(temperature_str);

    if (length_str == "in" || length_str == "ft") {
        system.style = Style::Imperial;
    } else {
        system.style = Style::Metric;
    }

    if (length_unit.type != UnitType::length) {
        throw UnitParseError("'" + length_str + "' is not a unit of length.");
    }
    system.length_unit_in_si = length_unit.unit_in_si;

    if (time_unit.type != UnitType::time) {
        throw UnitParseError("'" + time_str + "' is not a unit of time.");
    }
    system.time_unit_in_si = time_unit.unit_in_si;

    if (mass_or_force_unit.type == UnitType::mass) {
        system.mass_unit_in_si = mass_or_force_unit.unit_in_si;
    } else if (mass_or_force_unit.type == UnitType::force) {
        system.mass_unit_in_si = mass_or_force_unit.unit_in_si
            / (system.length_unit_in_si / pow(system.time_unit_in_si, 2));
    } else {
        throw UnitParseError("'" + mass_or_force_str +
            "' is not a unit of mass or force.");
    }

    if (temperature_unit.type != UnitType::temperature) {
        throw UnitParseError("'" + temperature_str +
            "' is not a unit of temperature.");
    }
    system.temperature_unit_in_si = temperature_unit.unit_in_si;
    system.temperature_zero_in_si = temperature_unit.zero_in_si;

    return system;
}

double UnitSystem::convert_from_unit(double value_in_unit, Unit unit) const {
    double value = value_in_unit;
    value *= unit.unit_in_si;
    if (unit.type.arbitrary_zero) {
        assert(unit.type == UnitType::celsius_fahrenheit_temperature);
        value += unit.zero_in_si;
        value -= temperature_zero_in_si;
    }
    value /= pow(length_unit_in_si, unit.type.length_power);
    value /= pow(mass_unit_in_si, unit.type.mass_power);
    value /= pow(time_unit_in_si, unit.type.time_power);
    value /= pow(temperature_unit_in_si,
        unit.type.thermodynamic_temperature_power);
    return value;
}

double UnitSystem::convert_to_unit(double value_in_system, Unit unit) const {
    double value = value_in_system;
    value *= pow(length_unit_in_si, unit.type.length_power);
    value *= pow(mass_unit_in_si, unit.type.mass_power);
    value *= pow(time_unit_in_si, unit.type.time_power);
    value *= pow(temperature_unit_in_si,
        unit.type.thermodynamic_temperature_power);
    if (unit.type.arbitrary_zero) {
        assert(unit.type == UnitType::celsius_fahrenheit_temperature);
        value -= temperature_zero_in_si;
        value += unit.zero_in_si;
    }
    value /= unit.unit_in_si;
    return value;
}

Unit UnitSystem::choose_unit(
    UnitType type,
    double value_for_scale_in_system
) const {
    double value_for_scale_in_si =
        convert_to_unit(value_for_scale_in_system, Unit(type, 1.0));
    if (style == Style::Imperial) {

    }
}

} /* namespace os2cx */
