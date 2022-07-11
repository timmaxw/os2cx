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
    run.run();

    if (run.geometry == nullptr) {
        throw BeaconError();
    }
    return recover_beacon(*run.geometry);
}

void expect_equal_affine_transform(
    AffineTransform expected,
    AffineTransform actual
) {
    EXPECT_NEAR(expected.vector.x, actual.vector.x, 1e-6);
    EXPECT_NEAR(expected.vector.y, actual.vector.y, 1e-6);
    EXPECT_NEAR(expected.vector.z, actual.vector.z, 1e-6);
    for (int i = 0; i < 3; ++i) {
        EXPECT_NEAR(expected.matrix.cols[i].x, actual.matrix.cols[i].x, 1e-6);
        EXPECT_NEAR(expected.matrix.cols[i].y, actual.matrix.cols[i].y, 1e-6);
        EXPECT_NEAR(expected.matrix.cols[i].z, actual.matrix.cols[i].z, 1e-6);
    }
}

TEST(BeaconTest, Identity) {
    AffineTransform expected;
    expected.vector = LengthVector(0, 0, 0);
    expected.matrix = Matrix::identity();
    AffineTransform actual = try_beacon(
        "__os2cx_beacon();");
    expect_equal_affine_transform(expected, actual);
}

TEST(BeaconTest, Rotate) {
    AffineTransform expected;
    expected.vector = LengthVector(0, 0, 0);
    expected.matrix = Matrix::rotation(2, M_PI/2);
    AffineTransform actual = try_beacon(
        "rotate([0, 0, 90]) __os2cx_beacon();");
    expect_equal_affine_transform(expected, actual);
}

TEST(BeaconTest, Translate) {
    AffineTransform expected;
    expected.vector = LengthVector(12, 34, 56);
    expected.matrix = Matrix::identity();
    AffineTransform actual = try_beacon(
        "translate([12, 34, 56]) __os2cx_beacon();");
    expect_equal_affine_transform(expected, actual);
}

TEST(BeaconTest, RotateTranslate) {
    AffineTransform expected;
    expected.vector = LengthVector(-34, 12, 56);
    expected.matrix = Matrix::rotation(2, M_PI/2);
    AffineTransform actual = try_beacon(
        "rotate([0, 0, 90]) translate([12, 34, 56]) __os2cx_beacon();");
    expect_equal_affine_transform(expected, actual);
}

TEST(BeaconTest, Scale) {
    AffineTransform expected;
    expected.vector = LengthVector(0, 0, 0);
    expected.matrix = Matrix::scale(12, 34, 56);
    AffineTransform actual = try_beacon(
        "scale([12, 34, 56]) __os2cx_beacon();");
    expect_equal_affine_transform(expected, actual);
}

TEST(BeaconTest, Mirror) {
    AffineTransform expected;
    expected.vector = LengthVector(0, 0, 0);
    expected.matrix = Matrix::scale(-1, 1, 1);
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
