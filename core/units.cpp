#include "units.hpp"

#include <assert.h>
#include <math.h>

#include <set>
#include <sstream>

namespace os2cx {

Unit unit_m() {
    return Unit("m", UnitType::Length, 1, Unit::Metric);
}
Unit unit_in() {
    return Unit("in", UnitType::Length, 2.54e-2, Unit::Imperial);
}
Unit unit_g() {
    return Unit("g", UnitType::Mass, 1e-3, Unit::Metric);
}
Unit unit_lbm() {
    return Unit("lbm", UnitType::Mass, 4.5359237e-1, Unit::Imperial);
}
Unit unit_s() {
    return Unit("s", UnitType::Time, 1, Unit::MetricAndImperial);
}
Unit unit_min() {
    return Unit("min", UnitType::Time, 60, Unit::MetricAndImperial);
}
Unit unit_h() {
    return Unit("h", UnitType::Time, 60 * 60, Unit::MetricAndImperial);
}
Unit unit_N() {
    return Unit("N", UnitType::Force, 1, Unit::Metric);
}
Unit unit_lbf() {
    return Unit("lbf", UnitType::Force, 4.448222e+0, Unit::Imperial);
}
Unit unit_Pa() {
    return Unit("Pa", UnitType::Pressure, 1, Unit::Metric);
}

Unit unit_power(const Unit &base, int power, UnitType type) {
    return Unit(
        base.name + "^" + std::to_string(power),
        type,
        pow(base.unit_in_si, power),
        base.style);
}

Unit unit_ratio(const Unit &num, const Unit &denom, UnitType type) {
    return Unit(
        num.name + "/" + denom.name,
        type,
        num.unit_in_si / denom.unit_in_si,
        static_cast<Unit::Style>(num.style & denom.style));
}

bool strip_suffix(
    std::string *word,
    const std::string &suffix
) {
    if (word->length() < suffix.length()) {
        return false;
    }
    if (word->substr(word->length() - suffix.length()) != suffix) {
        return false;
    }
    *word = word->substr(0, word->length() - suffix.length());
    return true;
}

bool parse_unit_si_prefixed(
    const std::string &name,
    const Unit &base,
    Unit *output
) {
    static const std::map<std::string, double> prefix_exponents_by_name {
        {"y",  1e-24}, {"z", 1e-21}, {"a", 1e-18}, {"f", 1e-15}, {"p", 1e-12},
        {"n",  1e-09}, {"u", 1e-06}, {"m", 1e-03}, {"c", 1e-02}, {"d", 1e-01},
        {"",   1e+00},
        {"da", 1e+01}, {"h", 1e+02}, {"k", 1e+03}, {"M", 1e+06}, {"G", 1e+09},
        {"T",  1e+12}, {"P", 1e+15}, {"E", 1e+18}, {"Z", 1e+21}, {"Y", 1e+24}
    };
    std::string prefix_name = name;
    if (!strip_suffix(&prefix_name, base.name)) return false;
    auto it = prefix_exponents_by_name.find(prefix_name);
    if (it == prefix_exponents_by_name.end()) return false;
    *output = Unit(name, base.type, base.unit_in_si * it->second, base.style);
    return true;
}

bool parse_unit_power(
    const std::string &name,
    UnitType type,
    int power,
    UnitType output_type,
    Unit *output
) {
    std::string base_name = name;
    if (power != 1) {
        if (!strip_suffix(&base_name, "^" + std::to_string(power))) {
            return false;
        }
    }
    Unit base;
    try {
        base = Unit::from_name(type, base_name);
    } catch (UnitParseError) {
        return false;
    }
    double unit_in_si = pow(base.unit_in_si, power);
    *output = Unit(name, output_type, unit_in_si, base.style);
    return true;
}

bool parse_unit_ratio(
    const std::string &name,
    UnitType num_type,
    UnitType denom_type,
    UnitType output_type,
    Unit *output
) {
    int slash_pos = name.find('/');
    std::string num_name = name.substr(0, slash_pos);
    std::string denom_name = name.substr(slash_pos + 1);
    Unit num, denom;
    try {
        num = Unit::from_name(num_type, num_name);
        denom = Unit::from_name(denom_type, denom_name);
    } catch (UnitParseError) {
        return false;
    }
    double unit_in_si = num.unit_in_si / denom.unit_in_si;
    Unit::Style style = static_cast<Unit::Style>(num.style & denom.style);
    *output = Unit(name, output_type, unit_in_si, style);
    return true;
}

Unit Unit::from_name(UnitType type, const std::string &name) {
    Unit output;
    switch (type) {
    case UnitType::Length:
        if (parse_unit_si_prefixed(name, unit_m(), &output)) return output;
        if (name == "in") return unit_in();
        throw UnitParseError("'" + name + "' is not a valid length unit. " \
            "Valid length units are 'm' (with optional prefix) or 'in'.");
    case UnitType::Mass:
        if (parse_unit_si_prefixed(name, unit_g(), &output)) return output;
        if (name == "lbm") return unit_lbm();
        throw UnitParseError("'" + name + "' is not a valid mass unit. " \
            "Valid mass units are 'g' (with optional prefix) or 'lbm'.");
    case UnitType::Time:
        if (parse_unit_si_prefixed(name, unit_s(), &output)) return output;
        if (name == "min") return unit_min();
        if (name == "h") return unit_h();
        throw UnitParseError("'" + name + "' is not a valid time unit. " \
            "Valid time units are 's' (with optional prefix), 'min', or 'h'.");
    case UnitType::Area:
        if (parse_unit_power(
                name,
                UnitType::Length,
                2,
                UnitType::Area,
                &output)) {
            return output;
        }
        throw UnitParseError("'" + name + "' is not a valid area unit. " \
            "Valid area units are of the form '<length>^2'.");
    case UnitType::Force:
        if (parse_unit_si_prefixed(name, unit_N(), &output)) return output;
        if (name == "lbf") return unit_lbf();
        throw UnitParseError("'" + name + "' is not a valid force unit. " \
            "Valid force units are 'N' (with optional prefix) or 'lbf'.");
    case UnitType::Pressure:
        if (parse_unit_si_prefixed(name, unit_Pa(), &output)) return output;
        if (parse_unit_ratio(
                name,
                UnitType::Force,
                UnitType::Area,
                UnitType::Pressure,
                &output)) {
            if (output.style == Unit::InvalidStyle) {
                throw UnitParseError("'" + name + "' mixes metric and " \
                    "imperial units. Please don't mix metric and imperial.");
            }
            return output;
        }
        throw UnitParseError("'" + name + "' is not a valid pressure unit. " \
            "Valid pressure units are 'Pa' (with optional prefix), or " \
            "compound units of the form '<force>/<length>^2'.");
    }
    assert(false);
}

UnitSystem::UnitSystem(
    const std::string &length_name,
    const std::string &mass_or_force_name,
    const std::string &time_name
) {
    length_unit = Unit::from_name(UnitType::Length, length_name);
    assert(length_unit.style == Unit::Metric
        || length_unit.style == Unit::Imperial);
    style = length_unit.style;
    units_in_si[UnitType::Length] = length_unit.unit_in_si;

    Unit time_unit = Unit::from_name(UnitType::Time, time_name);
    units_in_si[UnitType::Time] = time_unit.unit_in_si;

    try {
        Unit mass_unit = Unit::from_name(UnitType::Mass, mass_or_force_name);
        units_in_si[UnitType::Mass] = mass_unit.unit_in_si;
        units_in_si[UnitType::Force] = mass_unit.unit_in_si
            * length_unit.unit_in_si / pow(time_unit.unit_in_si, 2);
    } catch (UnitParseError) {
        try {
            Unit force_unit = Unit::from_name(
                UnitType::Force, mass_or_force_name);
            units_in_si[UnitType::Force] = force_unit.unit_in_si;
            units_in_si[UnitType::Mass] = force_unit.unit_in_si
                / length_unit.unit_in_si * pow(time_unit.unit_in_si, 2);
        } catch (UnitParseError) {
            /* This error message should ideally give a list of the valid
            units... */
            throw UnitParseError("'" + mass_or_force_name + "' is not a " \
                "valid unit of either mass or force.");
        }
    }

    units_in_si[UnitType::Area] = pow(length_unit.unit_in_si, 2);
    units_in_si[UnitType::Pressure] =
        units_in_si[UnitType::Force] / units_in_si[UnitType::Area];
}

double UnitSystem::unit_to_system(const Unit &unit, double unit_value) const {
    double si_value = unit_value * unit.unit_in_si;
    double system_value = si_value / units_in_si.at(unit.type);
    return system_value;
}

double UnitSystem::system_to_unit(const Unit &unit, double system_value) const {
    double si_value = system_value * units_in_si.at(unit.type);
    double unit_value = si_value / unit.unit_in_si;
    return unit_value;
}

Unit suggest_unit_si_prefixed(const Unit &base, double si_value_for_scale) {
    /* Omit uncommon SI prefixes because we don't want to suggest them as
    output, even though we accept them as input. */
    static const std::map<double, std::string> prefix_names_by_exponent {
        {1e-09, "n"}, {1e-06, "u"}, {1e-03, "m"},
        {1e+00, "" },
        {1e+03, "k"}, {1e+06, "M"}, {1e+09, "G"}
    };
    auto it = prefix_names_by_exponent.upper_bound(si_value_for_scale);
    std::pair<double, std::string> factor_and_name;
    if (it == prefix_names_by_exponent.end()) {
        factor_and_name = *prefix_names_by_exponent.rbegin();
    } else if (it != prefix_names_by_exponent.begin()) {
        factor_and_name = *(--it);
    } else {
        factor_and_name = *it;
    }
    return Unit(
        factor_and_name.second + base.name,
        base.type,
        base.unit_in_si * factor_and_name.first,
        base.style);
}

Unit UnitSystem::suggest_unit(
    UnitType type,
    double system_value_for_scale
) const {
    double si_value_for_scale =
        std::abs(system_value_for_scale * units_in_si.at(type));
    switch (type) {
    case UnitType::Length:
        if (style == Unit::Metric) {
            return suggest_unit_si_prefixed(unit_m(), si_value_for_scale);
        } else {
            return unit_in();
        }
    case UnitType::Mass:
        if (style == Unit::Metric) {
            return suggest_unit_si_prefixed(unit_g(), si_value_for_scale * 1e3);
        } else {
            return unit_lbm();
        }
    case UnitType::Time:
        if (si_value_for_scale >= 60 * 60) {
            return unit_h();
        } else if (si_value_for_scale >= 60) {
            return unit_min();
        } else {
            return suggest_unit_si_prefixed(unit_s(), si_value_for_scale);
        }
    case UnitType::Area:
        return unit_power(length_unit, 2, UnitType::Area);
    case UnitType::Force:
        if (style == Unit::Metric) {
            return suggest_unit_si_prefixed(unit_N(), si_value_for_scale);
        } else {
            return unit_lbf();
        }
    case UnitType::Pressure:
        if (style == Unit::Metric) {
            return suggest_unit_si_prefixed(unit_Pa(), si_value_for_scale);
        } else {
            return unit_ratio(
                unit_lbf(),
                unit_power(unit_in(), 2, UnitType::Area),
                UnitType::Pressure);
        }
    default:
        assert(false);
    }
}

} /* namespace os2cx */
