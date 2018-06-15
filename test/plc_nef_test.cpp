#include <fstream>

#include <gtest/gtest.h>

#include "plc_nef.hpp"

namespace os2cx {

PlcNef3::Bitset bitset_zero() {
    return PlcNef3::Bitset();
}

PlcNef3::Bitset bitset_one() {
    PlcNef3::Bitset bitset;
    bitset.set();
    return bitset;
}

const PlcNef3 region_u = PlcNef3::from_poly(Poly3::box(0, 0, 0, 2, 2, 2));
const PlcNef3 region_v = PlcNef3::from_poly(Poly3::box(0, 0, 1, 2, 2, 3));
const Point point_in_u = Point::raw(1, 1, 0.5);
const Point point_in_v = Point::raw(1, 1, 2.5);
const Point point_in_uv = Point::raw(1, 1, 1.5);
const Point point_on_u_face = Point::raw(1, 1, 0);
const Point point_on_u_edge = Point::raw(1, 0, 0);
const Point point_on_u_vertex = Point::raw(0, 0, 0);
const Point point_outside = Point::raw(-1, -1, -1);

TEST(PlcNefTest, FromPoly) {
    EXPECT_EQ(bitset_one(), region_u.get_bitset(point_in_u));
    EXPECT_EQ(bitset_zero(), region_u.get_bitset(point_outside));
}

TEST(PlcNefTest, Map) {
    Plc3::Bitset bitset_volume, bitset_face, bitset_edge, bitset_vertex;
    bitset_volume.set(0);
    bitset_face.set(1);
    bitset_edge.set(2);
    bitset_vertex.set(3);

    PlcNef3 mapped = region_u.clone();
    mapped.map_volumes([&](Plc3::Bitset prev) {
        if (prev == bitset_one()) {
            return bitset_volume;
        } else {
            EXPECT_EQ(bitset_zero(), prev);
            return bitset_zero();
        }
    });
    mapped.map_faces(
    [&](Plc3::Bitset prev, Plc3::Bitset, Plc3::Bitset, PureVector) {
        EXPECT_EQ(bitset_one(), prev);
        return bitset_face;
    });
    mapped.map_edges([&](Plc3::Bitset prev) {
        EXPECT_EQ(bitset_one(), prev);
        return bitset_edge;
    });
    mapped.map_vertices([&](Plc3::Bitset prev) {
        EXPECT_EQ(bitset_one(), prev);
        return bitset_vertex;
    });

    EXPECT_EQ(bitset_volume, mapped.get_bitset(point_in_u));
    EXPECT_EQ(bitset_face, mapped.get_bitset(point_on_u_face));
    EXPECT_EQ(bitset_edge, mapped.get_bitset(point_on_u_edge));
    EXPECT_EQ(bitset_vertex, mapped.get_bitset(point_on_u_vertex));
    EXPECT_EQ(bitset_zero(), mapped.get_bitset(point_outside));

    mapped.map_volumes([&](Plc3::Bitset prev) {
        if (prev != bitset_zero()) {
            EXPECT_EQ(bitset_volume, prev);
        }
        return prev;
    });
    mapped.map_faces(
    [&](Plc3::Bitset prev, Plc3::Bitset, Plc3::Bitset, PureVector) {
        EXPECT_EQ(bitset_face, prev);
        return prev;
    });
    mapped.map_edges([&](Plc3::Bitset prev) {
        EXPECT_EQ(bitset_edge, prev);
        return prev;
    });
    mapped.map_vertices([&](Plc3::Bitset prev) {
        EXPECT_EQ(bitset_vertex, prev);
        return prev;
    });
}

TEST(PlcNefTest, BinaryOps) {
    PlcNef3::Bitset bitset_u(0x00FF);
    PlcNef3::Bitset bitset_v(0x0FF0);
    PlcNef3 u = region_u.clone();
    u.binarize(bitset_u, bitset_zero());
    PlcNef3 v = region_v.clone();
    v.binarize(bitset_v, bitset_zero());

    PlcNef3 u_or_v = u.binary_or(v);
    EXPECT_EQ(bitset_zero(), u_or_v.get_bitset(point_outside));
    EXPECT_EQ(bitset_u, u_or_v.get_bitset(point_in_u));
    EXPECT_EQ(bitset_v, u_or_v.get_bitset(point_in_v));
    EXPECT_EQ(bitset_u | bitset_v, u_or_v.get_bitset(point_in_uv));

    PlcNef3 u_and_v = u.binary_and(v);
    EXPECT_EQ(bitset_zero(), u_and_v.get_bitset(point_outside));
    EXPECT_EQ(bitset_zero(), u_and_v.get_bitset(point_in_u));
    EXPECT_EQ(bitset_zero(), u_and_v.get_bitset(point_in_v));
    EXPECT_EQ(bitset_u & bitset_v, u_and_v.get_bitset(point_in_uv));

    PlcNef3 u_and_not_v = u.binary_and_not(v);
    EXPECT_EQ(bitset_zero(), u_and_not_v.get_bitset(point_outside));
    EXPECT_EQ(bitset_u, u_and_not_v.get_bitset(point_in_u));
    EXPECT_EQ(bitset_zero(), u_and_not_v.get_bitset(point_in_v));
    EXPECT_EQ(bitset_u & ~bitset_v, u_and_not_v.get_bitset(point_in_uv));

    PlcNef3 u_xor_v = u.binary_xor(v);
    EXPECT_EQ(bitset_zero(), u_xor_v.get_bitset(point_outside));
    EXPECT_EQ(bitset_u, u_xor_v.get_bitset(point_in_u));
    EXPECT_EQ(bitset_v, u_xor_v.get_bitset(point_in_v));
    EXPECT_EQ(bitset_u ^ bitset_v, u_xor_v.get_bitset(point_in_uv));
}

} /* namespace os2cx */

