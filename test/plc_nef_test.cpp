#include <fstream>

#include <gtest/gtest.h>

#include "plc_nef.hpp"

namespace os2cx {

AttrBitset attrs_zero() {
    return AttrBitset();
}

AttrBitset attrs_one() {
    AttrBitset attrs;
    attrs.set();
    return attrs;
}

const PlcNef3 region_u =
    PlcNef3::from_poly(Poly3::from_box(Box(0, 0, 0, 2, 2, 2)));
const PlcNef3 region_v =
    PlcNef3::from_poly(Poly3::from_box(Box(0, 0, 1, 2, 2, 3)));
const Point point_in_u(1, 1, 0.5);
const Point point_in_v(1, 1, 2.5);
const Point point_in_uv(1, 1, 1.5);
const Point point_on_u_face(1, 1, 0);
const Point point_on_u_edge(1, 0, 0);
const Point point_on_u_vertex(0, 0, 0);
const Point point_outside(-1, -1, -1);

TEST(PlcNefTest, FromPoly) {
    EXPECT_EQ(attrs_one(), region_u.get_attrs(point_in_u));
    EXPECT_EQ(attrs_zero(), region_u.get_attrs(point_outside));
}

TEST(PlcNefTest, MultiPart) {
    PlcNef3 two_boxes = PlcNef3::from_poly(Poly3::from_boxes(
        {Box(0, 0, 0, 1, 1, 1), Box(2, 0, 0, 3, 1, 1)},
        {false, false}
    ));

    EXPECT_EQ(attrs_one(), two_boxes.get_attrs(Point(0.5, 0.5, 0.5)));
    EXPECT_EQ(attrs_zero(), two_boxes.get_attrs(Point(1.5, 0.5, 0.5)));
    EXPECT_EQ(attrs_one(), two_boxes.get_attrs(Point(2.5, 0.5, 0.5)));
    EXPECT_EQ(attrs_zero(), two_boxes.get_attrs(Point(3.5, 0.5, 0.5)));
}

TEST(PlcNefTest, Hole) {
    PlcNef3 box_with_hole = PlcNef3::from_poly(Poly3::from_boxes(
        {Box(0, 0, 0, 3, 3, 3), Box(1, 1, 1, 2, 2, 2)},
        {false, true}
    ));

    EXPECT_EQ(attrs_one(), box_with_hole.get_attrs(Point(0.5, 0.5, 0.5)));
    EXPECT_EQ(attrs_zero(), box_with_hole.get_attrs(Point(1.5, 1.5, 1.5)));
    EXPECT_EQ(attrs_one(), box_with_hole.get_attrs(Point(2.5, 2.5, 2.5)));
    EXPECT_EQ(attrs_zero(), box_with_hole.get_attrs(Point(3.5, 3.5, 3.5)));
}

TEST(PlcNefTest, Map) {
    AttrBitset attrs_volume, attrs_face, attrs_edge, attrs_vertex;
    attrs_volume.set(0);
    attrs_face.set(1);
    attrs_edge.set(2);
    attrs_vertex.set(3);

    PlcNef3 mapped = region_u.clone();
    mapped.map_volumes([&](AttrBitset prev) {
        if (prev == attrs_one()) {
            return attrs_volume;
        } else {
            EXPECT_EQ(attrs_zero(), prev);
            return attrs_zero();
        }
    });
    mapped.map_faces(
    [&](AttrBitset prev, AttrBitset, AttrBitset, Vector) {
        EXPECT_EQ(attrs_one(), prev);
        return attrs_face;
    });
    mapped.map_edges([&](AttrBitset prev) {
        EXPECT_EQ(attrs_one(), prev);
        return attrs_edge;
    });
    mapped.map_vertices([&](AttrBitset prev) {
        EXPECT_EQ(attrs_one(), prev);
        return attrs_vertex;
    });

    EXPECT_EQ(attrs_volume, mapped.get_attrs(point_in_u));
    EXPECT_EQ(attrs_face, mapped.get_attrs(point_on_u_face));
    EXPECT_EQ(attrs_edge, mapped.get_attrs(point_on_u_edge));
    EXPECT_EQ(attrs_vertex, mapped.get_attrs(point_on_u_vertex));
    EXPECT_EQ(attrs_zero(), mapped.get_attrs(point_outside));

    mapped.map_volumes([&](AttrBitset prev) {
        if (prev != attrs_zero()) {
            EXPECT_EQ(attrs_volume, prev);
        }
        return prev;
    });
    mapped.map_faces(
    [&](AttrBitset prev, AttrBitset, AttrBitset, Vector) {
        EXPECT_EQ(attrs_face, prev);
        return prev;
    });
    mapped.map_edges([&](AttrBitset prev) {
        EXPECT_EQ(attrs_edge, prev);
        return prev;
    });
    mapped.map_vertices([&](AttrBitset prev) {
        EXPECT_EQ(attrs_vertex, prev);
        return prev;
    });
}

TEST(PlcNefTest, BinaryOps) {
    AttrBitset attrs_u(0x00FF);
    AttrBitset attrs_v(0x0FF0);
    PlcNef3 u = region_u.clone();
    u.binarize(attrs_u, attrs_zero());
    PlcNef3 v = region_v.clone();
    v.binarize(attrs_v, attrs_zero());

    PlcNef3 u_or_v = u.binary_or(v);
    EXPECT_EQ(attrs_zero(), u_or_v.get_attrs(point_outside));
    EXPECT_EQ(attrs_u, u_or_v.get_attrs(point_in_u));
    EXPECT_EQ(attrs_v, u_or_v.get_attrs(point_in_v));
    EXPECT_EQ(attrs_u | attrs_v, u_or_v.get_attrs(point_in_uv));

    PlcNef3 u_and_v = u.binary_and(v);
    EXPECT_EQ(attrs_zero(), u_and_v.get_attrs(point_outside));
    EXPECT_EQ(attrs_zero(), u_and_v.get_attrs(point_in_u));
    EXPECT_EQ(attrs_zero(), u_and_v.get_attrs(point_in_v));
    EXPECT_EQ(attrs_u & attrs_v, u_and_v.get_attrs(point_in_uv));

    PlcNef3 u_and_not_v = u.binary_and_not(v);
    EXPECT_EQ(attrs_zero(), u_and_not_v.get_attrs(point_outside));
    EXPECT_EQ(attrs_u, u_and_not_v.get_attrs(point_in_u));
    EXPECT_EQ(attrs_zero(), u_and_not_v.get_attrs(point_in_v));
    EXPECT_EQ(attrs_u & ~attrs_v, u_and_not_v.get_attrs(point_in_uv));

    PlcNef3 u_xor_v = u.binary_xor(v);
    EXPECT_EQ(attrs_zero(), u_xor_v.get_attrs(point_outside));
    EXPECT_EQ(attrs_u, u_xor_v.get_attrs(point_in_u));
    EXPECT_EQ(attrs_v, u_xor_v.get_attrs(point_in_v));
    EXPECT_EQ(attrs_u ^ attrs_v, u_xor_v.get_attrs(point_in_uv));
}

} /* namespace os2cx */

