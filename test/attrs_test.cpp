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
    Mesh3 mesh = mesher_tetgen(plc, 0.1);
    ElementSet element_set = compute_element_set_from_plc_bit(
        plc_index,
        mesh,
        mesh.elements.key_begin(),
        mesh.elements.key_end(),
        bit_index_mask);

    Vector force_per_volume(1.234, 0, 0);
    ConcentratedLoad load = compute_load_from_element_set(
        mesh, element_set, force_per_volume, true);

    Vector total_force(0, 0, 0);
    for (const auto &pair : load.loads) {
        total_force += pair.second.force;
    }

    double expected_force = force_per_volume.x * Volume(1);
    double actual_force = total_force.x;
    EXPECT_NEAR(expected_force, actual_force, 1e-6);
}

TEST(AttrsTest, LoadSurface) {
    PlcNef3 solid_nef =
        compute_plc_nef_for_solid(Poly3::box(0, 0, 0, 1, 1, 2));
    Plc3::BitIndex bit_index_mask = bit_index_solid() + 1;
    compute_plc_nef_select_surface(
        &solid_nef,
        Poly3::box(-0.1, -0.1, 1.1, 1.1, 1, 2.1),
        Vector(0, 0, 1),
        180,
        bit_index_mask);
    Plc3 plc = plc_nef_to_plc(solid_nef);
    Plc3Index plc_index(&plc);
    Mesh3 mesh = mesher_tetgen(plc, 0.1);
    Mesh3Index mesh_index(&mesh);
    FaceSet face_set = compute_face_set_from_plc_bit(
        plc_index,
        mesh,
        mesh_index,
        mesh.elements.key_begin(),
        mesh.elements.key_end(),
        bit_index_mask);

    Vector force_per_area(1.234, 0, 0);
    ConcentratedLoad load = compute_load_from_face_set(
        mesh, face_set, force_per_area, true);

    Vector total_force(0, 0, 0);
    for (const auto &pair : load.loads) {
        total_force += pair.second.force;
    }

    double area = 5;
    double expected_force = force_per_area.x * area;
    double actual_force = total_force.x;
    EXPECT_NEAR(expected_force, actual_force, 1e-6);
}

} /* namespace os2cx */
