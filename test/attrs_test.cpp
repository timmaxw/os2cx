#include <gtest/gtest.h>

#include "attrs.hpp"
#include "mesher_tetgen.hpp"

namespace os2cx {

TEST(AttrsTest, LoadVolume) {
    Region3 solid = Region3::box(0, 0, 0, 1, 1, 2);
    Region3 mask = Region3::box(0, 0, 1, 1, 1, 3);
    RegionMap3 region_map;
    region_map_create(solid, {&mask}, &region_map);
    RegionMap3Index region_map_index(region_map);
    Mesh3 mesh = mesher_tetgen(region_map);

    ConcentratedLoad load;
    ForceDensityVector force_density =
        ForceDensityVector::raw(1.234, 0, 0);
    load_volume(
        region_map, region_map_index, mesh,
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
