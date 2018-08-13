#ifndef OS2CX_UNITS_HPP_
#define OS2CX_UNITS_HPP_

#include <map>
#include <stdexcept>
#include <string>

namespace os2cx {

enum class UnitType {
    Length,
    Mass,
    Time,

    Area,
    Force,
    Pressure
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

class UnitSystem {
public:
    UnitSystem() { }
    UnitSystem(
        const std::string &length_name,
        const std::string &mass_or_force_name,
        const std::string &time_name);
    double unit_to_system(const Unit &unit, double unit_value) const;
    double system_to_unit(const Unit &unit, double system_value) const;
    Unit suggest_unit(UnitType type, double system_value_for_scale) const;

private:
    std::map<UnitType, double> units_in_si;
    Unit::Style style;
    Unit length_unit;
};

} /* namespace os2cx */

#endif /* OS2CX_UNITS_HPP_ */
