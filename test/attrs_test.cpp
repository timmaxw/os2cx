#include <gtest/gtest.h>

#include "attrs.hpp"
#include "mesher_tetgen.hpp"

namespace os2cx {

TEST(AttrsTest, LoadVolume) {
    Poly3 solid = Poly3::box(0, 0, 0, 1, 1, 2);
    Poly3 mask_poly = Poly3::box(0, 0, 1, 1, 1, 3);
    Poly3MapVolumeMask mask;
    mask.poly = &mask_poly;
    std::set<Poly3Map::VolumeId> mask_volumes;
    Poly3Map poly3_map;
    poly3_map_create(solid, {mask}, &poly3_map, {&mask_volumes});
    Poly3MapIndex poly3_map_index(poly3_map);
    Mesh3 mesh = mesher_tetgen(poly3_map);
    ElementSet element_set = compute_element_set_from_mask(
        poly3_map_index,
        mesh,
        mesh.elements.key_begin(),
        mesh.elements.key_end(),
        mask_volumes);

    ForceDensityVector force_density =
        ForceDensityVector::raw(1.234, 0, 0);
    ConcentratedLoad load = compute_load_from_element_set(
        mesh, element_set, force_density);

    ForceVector total_force = ForceVector::zero();
    for (const auto &pair : load.loads) {
        total_force += pair.second.force;
    }

    Force expected_force = force_density.x * Volume(1);
    Force actual_force = total_force.x;
    EXPECT_NEAR(expected_force.val, actual_force.val, 1e-6);
}

} /* namespace os2cx */
