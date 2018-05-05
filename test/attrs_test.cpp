#include <gtest/gtest.h>

#include "attrs.hpp"
#include "mesher_tetgen.hpp"

namespace os2cx {

TEST(AttrsTest, LoadVolume) {
    Poly3 solid = Poly3::box(0, 0, 0, 1, 1, 2);
    Poly3 mask = Poly3::box(0, 0, 1, 1, 1, 3);
    Poly3Map poly3_map;
    poly3_map_create(solid, {&mask}, &poly3_map);
    Poly3MapIndex poly3_map_index(poly3_map);
    Mesh3 mesh = mesher_tetgen(poly3_map);

    ConcentratedLoad load;
    ForceDensityVector force_density =
        ForceDensityVector::raw(1.234, 0, 0);
    load_volume(
        poly3_map, poly3_map_index, mesh,
        &mask, force_density, &load);

    ForceVector total_force = ForceVector::zero();
    for (const auto &pair : load.loads) {
        total_force += pair.second.force;
    }

    std::cout << element_shape_tetrahedron4().volume_function << std::endl;

    Force expected_force = force_density.x * Volume(1);
    Force actual_force = total_force.x;
    EXPECT_NEAR(expected_force.val, actual_force.val, 1e-6);
}

} /* namespace os2cx */
