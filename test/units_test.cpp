#include <gtest/gtest.h>

#include "units.hpp"

namespace os2cx {

TEST(UnitsTest, ParseMetricUnits) {
    Unit meter = Unit::from_name(UnitType::Length, "m");
    EXPECT_FLOAT_EQ(1, meter.unit_in_si);
    Unit gram = Unit::from_name(UnitType::Mass, "g");
    EXPECT_FLOAT_EQ(1e-3, gram.unit_in_si);
    Unit kilogram = Unit::from_name(UnitType::Mass, "kg");
    EXPECT_FLOAT_EQ(1, kilogram.unit_in_si);
}

TEST(UnitsTest, ParseImperialUnits) {
    Unit inch = Unit::from_name(UnitType::Length, "in");
    EXPECT_FLOAT_EQ(2.54e-2, inch.unit_in_si);
    Unit pound_force = Unit::from_name(UnitType::Force, "lbf");
    EXPECT_FLOAT_EQ(4.448222e+0, pound_force.unit_in_si);
}

TEST(UnitsTest, ParsePressureRatio) {
    Unit psi = Unit::from_name(UnitType::Pressure, "lbf/in^2");
    EXPECT_FLOAT_EQ(6894.76, psi.unit_in_si);
    Unit megapascal = Unit::from_name(UnitType::Pressure, "N/mm^2");
    EXPECT_FLOAT_EQ(1e6, megapascal.unit_in_si);
}

TEST(UnitsTest, ConvertSystem) {
    UnitSystem sys("cm", "g", "s");
    double inch_to_cm = sys.unit_to_system(
        WithUnit<double>(1.0, Unit::from_name(UnitType::Length, "in")));
    EXPECT_FLOAT_EQ(2.54, inch_to_cm);
    WithUnit<double> back_to_inch = sys.system_to_unit(
        Unit::from_name(UnitType::Length, "in"), 2.54);
    EXPECT_FLOAT_EQ(1.0, back_to_inch.value_in_unit);
    EXPECT_EQ("in", back_to_inch.unit.name);
}

TEST(UnitsTest, SystemMassOrForce) {
    UnitSystem mass_system("cm", "g", "s");
    EXPECT_FLOAT_EQ(1.0e5, mass_system.unit_to_system(
        WithUnit<double>(1.0, Unit::from_name(UnitType::Force, "N"))));
    EXPECT_FLOAT_EQ(1.0e3, mass_system.unit_to_system(
        WithUnit<double>(1.0, Unit::from_name(UnitType::Mass, "kg"))));
    UnitSystem force_system("cm", "N", "s");
    EXPECT_FLOAT_EQ(1.0, force_system.unit_to_system(
        WithUnit<double>(1.0, Unit::from_name(UnitType::Force, "N"))));
    EXPECT_FLOAT_EQ(1.0e-2, force_system.unit_to_system(
        WithUnit<double>(1.0, Unit::from_name(UnitType::Mass, "kg"))));
}

void expect_suggestion(
    const UnitSystem &system,
    UnitType type,
    double scale_value,
    const std::string &expected_unit
) {
    Unit suggestion = system.suggest_unit(type, scale_value);
    EXPECT_EQ(expected_unit, suggestion.name);
    EXPECT_EQ(type, suggestion.type);
    Unit reparsed = Unit::from_name(type, suggestion.name);
    EXPECT_EQ(suggestion.unit_in_si, reparsed.unit_in_si);
}

TEST(UnitsTest, SuggestUnit) {
    UnitSystem metric_system("m", "kg", "s");
    UnitSystem imperial_system("in", "lbf", "s");

    expect_suggestion(metric_system, UnitType::Length, 1.0, "m");
    expect_suggestion(metric_system, UnitType::Length, 1234, "km");
    expect_suggestion(metric_system, UnitType::Length, 1e100, "Gm");
    expect_suggestion(metric_system, UnitType::Length, 1e-100, "nm");
    expect_suggestion(imperial_system, UnitType::Length, 1.0, "in");

    expect_suggestion(metric_system, UnitType::Mass, 1.0, "kg");
    expect_suggestion(metric_system, UnitType::Mass, 0.1, "g");
    expect_suggestion(metric_system, UnitType::Mass, 1e-5, "mg");
    expect_suggestion(imperial_system, UnitType::Mass, 1.0, "lbm");

    expect_suggestion(metric_system, UnitType::Time, 1.0, "s");
    expect_suggestion(metric_system, UnitType::Time, 1e-100, "ns");
    expect_suggestion(metric_system, UnitType::Time, 100, "min");
    expect_suggestion(metric_system, UnitType::Time, 100000, "h");

    expect_suggestion(metric_system, UnitType::Area, 1.0, "m^2");

    expect_suggestion(metric_system, UnitType::Force, 1.0, "N");
    expect_suggestion(imperial_system, UnitType::Force, 1.0, "lbf");

    expect_suggestion(metric_system, UnitType::ForceDensity, 1.0, "N/m^3");
    expect_suggestion(imperial_system, UnitType::ForceDensity, 1.0, "lbf/in^3");

    expect_suggestion(metric_system, UnitType::Pressure, 1.0, "Pa");
    expect_suggestion(imperial_system, UnitType::Pressure, 1.0, "lbf/in^2");

    expect_suggestion(metric_system, UnitType::Volume, 1.0, "m^3");
    expect_suggestion(imperial_system, UnitType::Volume, 1.0, "in^3");
}

} /* namespace os2cx */
