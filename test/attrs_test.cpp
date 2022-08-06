#include <gtest/gtest.h>

#include "compute_attrs.hpp"
#include "mesher_tetgen.hpp"
#include "plc_nef_to_plc.hpp"

namespace os2cx {

TEST(AttrsTest, LoadVolume) {
    PlcNef3 solid_nef =
        compute_plc_nef_for_solid(Poly3::from_box(Box(0, 0, 0, 1, 1, 2)));
    AttrBitIndex bit_index_mask = attr_bit_solid() + 1;
    compute_plc_nef_select_volume(
        &solid_nef, Poly3::from_box(Box(0, 0, 1, 1, 1, 3)), bit_index_mask);
    Plc3 plc = plc_nef_to_plc(solid_nef);
    Mesh3 mesh = mesher_tetgen(
        plc, 0.5, AttrOverrides<MaxElementSize>(), ElementType::C3D10);
    ElementSet element_set = compute_element_set_from_attr_bit(
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
        compute_plc_nef_for_solid(Poly3::from_box(Box(0, 0, 0, 1, 1, 2)));
    AttrBitIndex bit_index_mask = attr_bit_solid() + 1;
    compute_plc_nef_select_surface_external(
        &solid_nef,
        Poly3::from_box(Box(-0.1, -0.1, 1, 1.1, 1.1, 2.1)),
        Vector(0, 0, 1),
        180,
        bit_index_mask);
    Plc3 plc = plc_nef_to_plc(solid_nef);
    Mesh3 mesh = mesher_tetgen(
        plc, 0.5, AttrOverrides<MaxElementSize>(), ElementType::C3D10);
    FaceSet face_set = compute_face_set_from_attr_bit(
        mesh,
        mesh.elements.key_begin(),
        mesh.elements.key_end(),
        Vector(1, 0, 0),
        180,
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

TEST(AttrsTest, ComputeSlice) {
    /* Make a box that extends from Z=0 to Z=2, and slice it in half along the
    Z=1 plane */
    Box solid_box(0, 0, 0, 1, 1, 2);
    PlcNef3 solid_nef = compute_plc_nef_for_solid(Poly3::from_box(solid_box));
    AttrBitIndex bit_index_mask = attr_bit_solid() + 1;
    compute_plc_nef_select_surface_internal(
        &solid_nef,
        Poly3::from_box(Box(-0.1, -0.1, -0.1, 1.1, 1.1, 1.0)),
        Vector(0, 0, 1),
        45,
        bit_index_mask);
    Plc3 plc = plc_nef_to_plc(solid_nef);
    Mesh3 mesh = mesher_tetgen(
        plc, 0.5, AttrOverrides<MaxElementSize>(), ElementType::C3D10);
    FaceSet face_set = compute_face_set_from_attr_bit(
        mesh,
        mesh.elements.key_begin(),
        mesh.elements.key_end(),
        Vector(0, 0, 1),
        45,
        bit_index_mask);
    Slice slice = compute_slice(&mesh, face_set);

    /* Sort the elements into which half they ended up in */
    std::map<ElementId, int> which_half;
    Volume volume_of_half[2] = {Volume(0), Volume(0)};
    for (ElementId element_id = mesh.elements.key_begin();
            element_id != mesh.elements.key_end(); ++element_id) {
        Point center_of_mass;
        Volume volume;
        mesh.center_of_mass(
            mesh.elements[element_id], &center_of_mass, &volume);
        EXPECT_TRUE(solid_box.contains(center_of_mass));
        int half = (center_of_mass.z < 1.0) ? 0 : 1;
        which_half[element_id] = half;
        volume_of_half[half] += volume;
    }

    Mesh3Index mesh_index(mesh);
    for (ElementId element_id = mesh.elements.key_begin();
            element_id != mesh.elements.key_end(); ++element_id) {
        const Element3 &element = mesh.elements[element_id];
        const ElementTypeShape &shape = element_type_shape(element.type);
        for (int face = 0; face < static_cast<int>(shape.faces.size());
                ++face) {
            FaceId face_id_1(element_id, face);
            FaceId face_id_2 = mesh_index.matching_face(face_id_1);
            if (face_id_2 == FaceId::invalid()) {
                continue;
            }
            ElementId element_id_2 = face_id_2.element_id;

            /* Each element should only be connected to other elements in the
            same half */
            EXPECT_EQ(which_half.at(element_id), which_half.at(element_id_2));
        }
    }

    /* The volume of each half should be 1.0 */
    EXPECT_NEAR(1.0, volume_of_half[0], 1e-6);
    EXPECT_NEAR(1.0, volume_of_half[1], 1e-6);

    std::set<NodeId> sliced_nodes;
    for (Slice::Pair slice_pair : slice.pairs) {
        sliced_nodes.insert(slice_pair.nodes[0]);
        sliced_nodes.insert(slice_pair.nodes[1]);

        Point point1 = mesh.nodes[slice_pair.nodes[0]].point;
        Point point2 = mesh.nodes[slice_pair.nodes[1]].point;

        /* The two nodes in the pair should have the exact same coordinate */
        EXPECT_EQ(point1, point2);

        /* The normals should all be parallel to the Z-axis. (It could point in
        either direction, depending on the ordering of the points.) */
        EXPECT_NEAR(1.0, fabs(slice_pair.normal.z), 1e-6);
    }

    for (NodeId node_id = mesh.nodes.key_begin();
            node_id != mesh.nodes.key_end(); ++node_id) {
        Point point = mesh.nodes[node_id].point;
        if (sliced_nodes.count(node_id)) {
            /* Sliced nodes should all lie on the Z=1 plane */
            EXPECT_NEAR(1.0, point.z, 1e-6);
        } else {
            /* Non-sliced nodes should not lie on the Z=1 plane */
            EXPECT_NE(1.0, point.z);
        }
    }
}

} /* namespace os2cx */
