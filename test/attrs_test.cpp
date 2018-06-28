#include <gtest/gtest.h>

#include "attrs.hpp"
#include "mesher_tetgen.hpp"
#include "plc_nef_to_plc.hpp"

namespace os2cx {

TEST(AttrsTest, LoadVolume) {
    PlcNef3 solid_nef =
        compute_plc_nef_for_solid(Poly3::box(0, 0, 0, 1, 1, 2));
    Plc3::BitIndex bit_index_mask = bit_index_solid() + 1;
    compute_plc_nef_select_volume(
        &solid_nef, Poly3::box(0, 0, 1, 1, 1, 3), bit_index_mask);
    Plc3 plc = plc_nef_to_plc(solid_nef);
    Plc3Index plc_index(&plc);
    Mesh3 mesh = mesher_tetgen(plc);
    ElementSet element_set = compute_element_set_from_plc_bit(
        plc_index,
        mesh,
        mesh.elements.key_begin(),
        mesh.elements.key_end(),
        bit_index_mask);

    Vector force_density(1.234, 0, 0);
    ConcentratedLoad load = compute_load_from_element_set(
        mesh, element_set, force_density);

    Vector total_force(0, 0, 0);
    for (const auto &pair : load.loads) {
        total_force += pair.second.force;
    }

    double expected_force = force_density.x * Volume(1);
    double actual_force = total_force.x;
    EXPECT_NEAR(expected_force, actual_force, 1e-6);
}

} /* namespace os2cx */
