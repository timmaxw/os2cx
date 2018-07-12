#include <gtest/gtest.h>

#include "units.hpp"

namespace os2cx {

TEST(UnitsTest, ParseSimple) {
    Unit u = Unit::parse_compound("m");
    EXPECT_EQ(UnitType::length, u.type);
    EXPECT_EQ(1, u.unit_in_si);
    EXPECT_EQ(0, u.zero_in_si);
}

TEST(UnitsTest, ParseSquare) {
    Unit u = Unit::parse_compound("m^2");
    EXPECT_EQ(UnitType::area, u.type);
    EXPECT_EQ(1, u.unit_in_si);
    EXPECT_EQ(0, u.zero_in_si);
}

TEST(UnitsTest, ParsePrefix) {
    Unit u = Unit::parse_compound("mm");
    EXPECT_EQ(UnitType::length, u.type);
    EXPECT_EQ(1e-3, u.unit_in_si);
    EXPECT_EQ(0, u.zero_in_si);
}

TEST(UnitsTest, ParsePrefixSquare) {
    Unit u = Unit::parse_compound("mm^2");
    EXPECT_EQ(UnitType::area, u.type);
    EXPECT_EQ(1e-6, u.unit_in_si);
    EXPECT_EQ(0, u.zero_in_si);
}

TEST(UnitsTest, ParseNegativePower) {
    Unit u = Unit::parse_compound("s^-1");
    EXPECT_EQ(UnitType::frequency, u.type);
    EXPECT_EQ(1, u.unit_in_si);
    EXPECT_EQ(0, u.zero_in_si);
}

TEST(UnitsTest, ParseProduct) {
    Unit u = Unit::parse_compound("m^2 kg s^-2");
    EXPECT_EQ(UnitType::energy, u.type);
    EXPECT_EQ(1, u.unit_in_si);
    EXPECT_EQ(0, u.zero_in_si);
}

TEST(UnitsTest, ParseDimensionless) {
    Unit u = Unit::parse_compound("dimensionless");
    EXPECT_EQ(UnitType::dimensionless, u.type);
    EXPECT_EQ(1, u.unit_in_si);
    EXPECT_EQ(0, u.zero_in_si);
}

TEST(UnitsTest, ParseErrors) {
    EXPECT_THROW(Unit::parse_compound("blah"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound(""), UnitParseError);
    EXPECT_THROW(Unit::parse_compound(" "), UnitParseError);
    EXPECT_THROW(Unit::parse_compound(" m"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("m "), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("m^0"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("m^1"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("m^-"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("m^- m"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("m m"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("m  s"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("Gft"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("degC m"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("degC^2"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("m*s"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("m/s"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("m^101"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("m^9999999999999999999"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("m 123"), UnitParseError);
    EXPECT_THROW(Unit::parse_compound("1"), UnitParseError);
}

} /* namespace os2cx */
