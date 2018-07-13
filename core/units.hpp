#ifndef OS2CX_UNITS_HPP_
#define OS2CX_UNITS_HPP_

#include <pair>
#include <stdexcept>
#include <string>

namespace os2cx {

class UnitType {
public:
    static UnitType dimensionless();

    static UnitType length();
    static UnitType mass();
    static UnitType time();
    static UnitType electric_current();
    static UnitType thermodynamic_temperature();
    static UnitType amount_of_substance();
    static UnitType luminous_intensity();

    static UnitType acceleration();
    static UnitType area();
    static UnitType capacitance();
    static UnitType density();
    static UnitType dynamic_viscosity();
    static UnitType electric_charge();
    static UnitType electric_conductance();
    static UnitType electric_potential();
    static UnitType electric_resistance();
    static UnitType energy();
    static UnitType force();
    static UnitType frequency();
    static UnitType inductance();
    static UnitType kinematic_viscosity();
    static UnitType magnetic_flux();
    static UnitType magnetic_flux_density();
    static UnitType power();
    static UnitType pressure();
    static UnitType speed();
    static UnitType volume();

    static UnitType celsius_fahrenheit_temperature();

    UnitType(
        int lp, int mp, int tp, int ecp, int ttp, int aosp, int lip,
        bool az = false);

    bool operator==(const UnitType &other) const;
    bool operator!=(const UnitType &other) const;

    int length_power;
    int mass_power;
    int time_power;
    int electric_current_power;
    int thermodynamic_temperature_power;
    int amount_of_substance_power;
    int luminous_intensity_power;

    /* 'arbitrary_zero' is true for Celsius and Fahrenheit, false for Kelvin and
    for non-temperature units. */
    bool arbitrary_zero;
};

class UnitParseError : public std::invalid_argument {
public:
    UnitParseError(const std::string &msg) : std::invalid_argument(msg) { }
};

/* Unit represents a unit of measure, like "inches" or "gigapascals". */
class Unit {
public:
    /* Invariants:
    parse_simple(str).abbrev == str       [if it doesn't throw an exception]
    parse_compound(str).abbrev == str     [if it doesn't throw an exception]
    parse_simple(unit.abbrev) == unit     [if it doesn't throw an exception]
    parse_compound(unit.abbrev) == unit   [will never throw an exception]
    */
    static Unit parse_simple(const std::string &str);
    static Unit parse_compound(const std::string &str);

    Unit(const std::string &a, UnitType t, double uis, double zis = 0.0) :
        abbrev(n), type(t), unit_in_si(uis), zero_in_si(zis) { }

    /* Note that multiple equivalent units can have the same 'abbrev'; for
    example, 'mL' and 'cm^3'. */
    std::string abbrev;

    UnitType type;
    double unit_in_si;
    double zero_in_si; /* Always zero except for Celsius and Fahrenheit */
};

class UnitSystem {
public:
    enum class Style {Metric, Imperial};

    static UnitSystem parse(
        const std::string &length_str,
        const std::string &mass_or_force_str,
        const std::string &time_str,
        const std::string &temperature_str);

    double convert_from_unit(double value_in_unit, Unit unit) const;
    double convert_to_unit(double value_in_unit, Unit unit) const;

    Unit choose_unit(UnitType type, double value_for_scale_in_system) const;

    Style style;
    double length_unit_in_si;
    double mass_unit_in_si;
    double time_unit_in_si;
    double temperature_unit_in_si;
    double temperature_zero_in_si;
};

} /* namespace os2cx */

#endif /* OS2CX_UNITS_HPP_ */
