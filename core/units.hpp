#ifndef OS2CX_UNITS_HPP_
#define OS2CX_UNITS_HPP_

#include <map>
#include <stdexcept>
#include <string>

#include "calc.hpp"

namespace os2cx {

enum class UnitType {
    Dimensionless,

    Length,
    Mass,
    Time,

    Area,
    Density,
    Force,
    ForcePerVolume,
    Pressure,
    Volume
};

class UnitParseError : public std::invalid_argument {
public:
    UnitParseError(const std::string &msg) : std::invalid_argument(msg) { }
};

class Unit {
public:
    enum Style {
        InvalidStyle = 0,
        Metric = 1,
        Imperial = 2,
        MetricAndImperial = Metric | Imperial
    };

    /* Invariants:
    Unit::from_name(type, name).name == name [if it doesn't throw]
    Unit::from_name(type, name).type == type [if it doesn't throw]
    Unit::from_name(unit.type, unit.name) == unit [will never throw]
    */
    static Unit from_name(UnitType type, const std::string &name);

    Unit() { }
    Unit(const std::string &n, UnitType t, double uis, Style s) :
        name(n), type(t), unit_in_si(uis), style(s) { }

    std::string name;
    UnitType type;
    double unit_in_si;
    Style style;
};

/* Every project has a UnitSystem. Physical quantities in the project are
normally stored as 'doubles' or a variety of other types, which are to be
interpreted according to the unit system. For example, in a Nef polyhedron, the
coordinates are denominated in the unit system. The same unit system is used in
the CalculiX calculation. So the only unit conversions take place when inputting
a value with an explicit unit from the OpenSCAD file, or when outputting a value
with an explicit unit in the GUI. */

/* In the rare cases when we work with values not in the normal unit system, we
wrap them in WithUnit<T> to avoid confusion. 'T' can be 'double', 'Vector', etc.
*/
template<class T>
class WithUnit {
public:
    WithUnit() { }
    WithUnit(const WithUnit &) = default;
    WithUnit(T viu, const Unit &u) : value_in_unit(viu), unit(u) { }

    /* 'x.value_in_unit' is in units of 'unit'. If you actually want the value
    in the project's unit system, instead call 'unit_system->unit_to_system(x)'.
    */
    T value_in_unit;

    Unit unit;
};

class UnitSystem {
public:
    UnitSystem() { }
    UnitSystem(
        const std::string &length_name,
        const std::string &mass_or_force_name,
        const std::string &time_name);

    double unit_to_system(const WithUnit<double> &value_with_unit) const;
    Vector unit_to_system(const WithUnit<Vector> &value_with_unit) const;

    WithUnit<double> system_to_unit(const Unit &unit, double value) const;
    WithUnit<Vector> system_to_unit(const Unit &unit, Vector value) const;

    Unit suggest_unit(UnitType type, double system_value_for_scale) const;

private:
    std::map<UnitType, double> units_in_si;
    Unit::Style style;
    Unit length_unit;
};

} /* namespace os2cx */

#endif /* OS2CX_UNITS_HPP_ */
