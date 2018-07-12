#ifndef OS2CX_UNITS_HPP_
#define OS2CX_UNITS_HPP_

#include <stdexcept>
#include <string>

namespace os2cx {

class UnitType {
public:
    static const UnitType dimensionless;

    static const UnitType length;
    static const UnitType mass;
    static const UnitType time;
    static const UnitType electric_current;
    static const UnitType temperature;
    static const UnitType amount_of_substance;
    static const UnitType luminous_intensity;

    static const UnitType acceleration;
    static const UnitType area;
    static const UnitType capacitance;
    static const UnitType density;
    static const UnitType dynamic_viscosity;
    static const UnitType electric_charge;
    static const UnitType electric_conductance;
    static const UnitType electric_potential;
    static const UnitType electric_resistance;
    static const UnitType energy;
    static const UnitType force;
    static const UnitType frequency;
    static const UnitType inductance;
    static const UnitType kinematic_viscosity;
    static const UnitType magnetic_flux;
    static const UnitType magnetic_flux_density;
    static const UnitType power;
    static const UnitType pressure;
    static const UnitType speed;
    static const UnitType volume;

    UnitType(int lp, int mp, int tp, int ecp, int tep, int aosp, int lip);

    bool operator==(const UnitType &other) const;
    bool operator!=(const UnitType &other) const;

    int length_power;
    int mass_power;
    int time_power;
    int electric_current_power;
    int temperature_power;
    int amount_of_substance_power;
    int luminous_intensity_power;
};

class UnitParseError : public std::invalid_argument {
public:
    UnitParseError(const std::string &msg) : std::invalid_argument(msg) { }
};

/* Unit represents a unit of measure, like "inches" or "gigapascals". The
constructor takes a string (typically user input) and parses it. Note that
there's no way to go in the opposite direction; you can't generate a string
representation of a Unit, because there are too many awkward corner cases. */
class Unit {
public:
    static Unit parse_simple(const std::string &str);
    static Unit parse_compound(const std::string &str);

    Unit(UnitType t, double uis, double zis = 0.0) :
        type(t), unit_in_si(uis), zero_in_si(zis) { }

    UnitType type;
    double unit_in_si;
    double zero_in_si; /* Always zero except for Celsius and Fahrenheit */
};

} /* namespace os2cx */

#endif /* OS2CX_UNITS_HPP_ */
