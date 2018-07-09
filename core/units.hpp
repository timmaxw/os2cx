#ifndef OS2CX_UNITS_HPP_
#define OS2CX_UNITS_HPP_

namespace os2cx {

class UnitType {
public:
    static const UnitType length, mass, time, temperature;
    static const UnitType electric_current, chemical_amount, luminous_intensity;

    static const UnitType area, volume, force, pressure;

    UnitType(int lp, int mp, int tp, int tmp, int ecp, int cap, int lip) :
        length_power(lp),
        mass_power(mp),
        time_power(tp),
        temperature_power(tmp),
        electric_current_power(ecp),
        chemical_amount_power(cap),
        luminous_intensity_power(lip)
        { }

    int length_power;
    int mass_power;
    int time_power;
    int temperature_power;
    int electric_current_power;
    int chemical_amount_power;
    int luminous_intensity_power;
};

/* Unit represents a unit of measure, like "inches" or "gigapascals". The
constructor takes a string (typically user input) and parses it. Note that
there's no way to go in the opposite direction; you can't generate a string
representation of a Unit, because there are too many awkward corner cases. */
class Unit {
public:
    static Unit parse(const std::string &str);

    Unit(UnitType t, double uis, double zis = 0.0) :
        type(t), unit_in_si(uis), zero_in_si(zis) { }

    UnitType type;
    double unit_in_si;
    double zero_in_si; /* Always zero except for Celsius and Fahrenheit */
};

/* UnitSystem represents a system of units that we're working in, such as
"meters, kilograms, seconds" or "feet, pounds, seconds". */
class UnitSystem {
public:
    /* convert_in() takes a given 'outside_value' denominated in the given
    'outside_unit', and converts that value into this system of units. */
    double convert_in(double outside_value, Unit outside_unit) const;

    /* convert_out() takes a given 'inside_value', having dimensions of
    'outside_unit.type', denominated in this system of units, and converts that
    value into the given 'outside_unit'. */
    double convert_out(double inside_value, Unit outside_unit) const;

    /* format() takes a given 'inside_value' having dimensions of 'type',
    denominated in this system of units, and returns a nicely formatted string
    representation. */
    std::string format(
        double inside_value,
        UnitType type,
        double max_value) const;

private:
    virtual void suggest_unit(
        UnitType type,
        double max_value,
        Unit *unit_out,
        std::string *unit_string_out
        ) const = 0;

    double length_unit_in_si;
    double mass_unit_in_si;
    double time_unit_in_si;
};

} /* namespace os2cx */

#endif /* OS2CX_UNITS_HPP_ */
