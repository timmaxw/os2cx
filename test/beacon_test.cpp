#include <fstream>

#include <gtest/gtest.h>

#include "beacon.hpp"
#include "openscad_run.hpp"
#include "util.hpp"

namespace os2cx {

AffineTransform try_beacon(const std::string &code) {
    TempDir temp_dir(
        "./test_openscad_runXXXXXX",
        TempDir::AutoCleanup::Yes);

    FilePath scad_path = temp_dir.path() + "/test.scad";
    std::ofstream stream(scad_path);
    stream << "include <../openscad2calculix.scad>;" << std::endl;
    stream << code << std::endl;
    stream.close();

    FilePath geometry_path = temp_dir.path() + "/test.off";
    std::map<std::string, OpenscadValue> defines;
    OpenscadRun run(scad_path, geometry_path, defines);
    run.wait();

    if (run.geometry == nullptr) {
        throw BeaconError();
    }
    return recover_beacon(*run.geometry);
}

void expect_equal_affine_transform(
    AffineTransform expected,
    AffineTransform actual
) {
    EXPECT_NEAR(expected.translation.x.val, actual.translation.x.val, 1e-6);
    EXPECT_NEAR(expected.translation.y.val, actual.translation.y.val, 1e-6);
    EXPECT_NEAR(expected.translation.z.val, actual.translation.z.val, 1e-6);
    for (int i = 0; i < 3; ++i) {
        EXPECT_NEAR(expected.matrix[i].x.val, actual.matrix[i].x.val, 1e-6);
        EXPECT_NEAR(expected.matrix[i].y.val, actual.matrix[i].y.val, 1e-6);
        EXPECT_NEAR(expected.matrix[i].z.val, actual.matrix[i].z.val, 1e-6);
    }
}

TEST(BeaconTest, Identity) {
    AffineTransform expected;
    expected.translation = LengthVector::raw(0, 0, 0);
    expected.matrix[0] = LengthVector::raw(1, 0, 0);
    expected.matrix[1] = LengthVector::raw(0, 1, 0);
    expected.matrix[2] = LengthVector::raw(0, 0, 1);
    AffineTransform actual = try_beacon(
        "__os2cx_beacon();");
    expect_equal_affine_transform(expected, actual);
}

TEST(BeaconTest, Rotate) {
    AffineTransform expected;
    expected.translation = LengthVector::raw(0, 0, 0);
    expected.matrix[0] = LengthVector::raw(0, 1, 0);
    expected.matrix[1] = LengthVector::raw(-1, 0, 0);
    expected.matrix[2] = LengthVector::raw(0, 0, 1);
    AffineTransform actual = try_beacon(
        "rotate([0, 0, 90]) __os2cx_beacon();");
    expect_equal_affine_transform(expected, actual);
}

TEST(BeaconTest, Translate) {
    AffineTransform expected;
    expected.translation = LengthVector::raw(12, 34, 56);
    expected.matrix[0] = LengthVector::raw(1, 0, 0);
    expected.matrix[1] = LengthVector::raw(0, 1, 0);
    expected.matrix[2] = LengthVector::raw(0, 0, 1);
    AffineTransform actual = try_beacon(
        "translate([12, 34, 56]) __os2cx_beacon();");
    expect_equal_affine_transform(expected, actual);
}

TEST(BeaconTest, RotateTranslate) {
    AffineTransform expected;
    expected.translation = LengthVector::raw(-34, 12, 56);
    expected.matrix[0] = LengthVector::raw(0, 1, 0);
    expected.matrix[1] = LengthVector::raw(-1, 0, 0);
    expected.matrix[2] = LengthVector::raw(0, 0, 1);
    AffineTransform actual = try_beacon(
        "rotate([0, 0, 90]) translate([12, 34, 56]) __os2cx_beacon();");
    expect_equal_affine_transform(expected, actual);
}

TEST(BeaconTest, Scale) {
    AffineTransform expected;
    expected.translation = LengthVector::raw(0, 0, 0);
    expected.matrix[0] = LengthVector::raw(12, 0, 0);
    expected.matrix[1] = LengthVector::raw(0, 34, 0);
    expected.matrix[2] = LengthVector::raw(0, 0, 56);
    AffineTransform actual = try_beacon(
        "scale([12, 34, 56]) __os2cx_beacon();");
    expect_equal_affine_transform(expected, actual);
}

TEST(BeaconTest, Mirror) {
    AffineTransform expected;
    expected.translation = LengthVector::raw(0, 0, 0);
    expected.matrix[0] = LengthVector::raw(-1, 0, 0);
    expected.matrix[1] = LengthVector::raw(0, 1, 0);
    expected.matrix[2] = LengthVector::raw(0, 0, 1);
    AffineTransform actual = try_beacon(
        "mirror([1, 0, 0]) __os2cx_beacon();");
    expect_equal_affine_transform(expected, actual);
}

TEST(BeaconTest, Corrupted) {
    EXPECT_THROW(try_beacon(""), BeaconError);
    EXPECT_THROW(try_beacon("cube([1,1,1]);"), BeaconError);
    EXPECT_THROW(try_beacon(
        "difference() { __os2cx_beacon(); sphere(0.5); };"
    ), BeaconError);
    EXPECT_THROW(try_beacon("hull() __os2cx_beacon();"), BeaconError);
    EXPECT_THROW(try_beacon(
        "minkowski() { __os2cx_beacon(); sphere(0.5); };"
    ), BeaconError);
}

} /* namespace os2cx */
