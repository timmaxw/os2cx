#include <fstream>

#include <gtest/gtest.h>

#include "plc.hpp"
#include "plc_index.hpp"
#include "plc_nef.hpp"
#include "plc_nef_to_plc.hpp"

namespace os2cx {

TEST(PlcTest, PlcNefToPlc) {
    Plc3::Bitset bitset_solid, bitset_mask;
    bitset_solid.set(0);
    bitset_mask.set(1);
    PlcNef3 solid = PlcNef3::from_poly(Poly3::box(0, 0, 0, 1, 1, 3));
    PlcNef3 mask = PlcNef3::from_poly(Poly3::box(-1, -1, 1, 2, 2, 2));
    solid.binarize(bitset_solid, Plc3::Bitset());
    mask.binarize(bitset_mask, Plc3::Bitset());
    PlcNef3 plc_nef = solid.binary_or(mask);
    plc_nef.map_everywhere([&](Plc3::Bitset bs, PlcNef3::FeatureType) {
        if (bs == bitset_mask) return Plc3::Bitset();
        else return bs;
    });

    Plc3 plc = plc_nef_to_plc(plc_nef);

    if (false) {
        plc.debug(std::cout);
    }

    ASSERT_EQ(16, plc.vertices.size());
    ASSERT_EQ(4, plc.volumes.size());
    ASSERT_EQ(5, plc.surfaces.size());
    ASSERT_EQ(2, plc.borders.size());

    Plc3Index ind(&plc);

    Plc3::VolumeId out = ind.volume_containing_point(Point(0.5, 0.5, -0.5));
    Plc3::VolumeId box1 = ind.volume_containing_point(Point(0.5, 0.5, 0.5));
    Plc3::VolumeId box2 = ind.volume_containing_point(Point(0.5, 0.5, 1.5));
    Plc3::VolumeId box3 = ind.volume_containing_point(Point(0.5, 0.5, 2.5));
    Plc3::VolumeId out2 = ind.volume_containing_point(Point(0.5, 0.5, 3.5));
    EXPECT_EQ(out, out2);
    EXPECT_EQ(plc.volume_outside, out);
    EXPECT_NE(box1, box3);
    EXPECT_EQ(Plc3::Bitset(), plc.volumes[out].bitset);
    EXPECT_EQ(bitset_solid, plc.volumes[box1].bitset);
    EXPECT_EQ(bitset_solid | bitset_mask, plc.volumes[box2].bitset);
    EXPECT_EQ(bitset_solid, plc.volumes[box3].bitset);

    Plc3::SurfaceId box1_out = ind.surface_closest_to_point(Point(0, 0, 0.5));
    Plc3::SurfaceId box2_out = ind.surface_closest_to_point(Point(0, 0, 1.5));
    Plc3::SurfaceId box3_out = ind.surface_closest_to_point(Point(0, 0, 2.5));
    Plc3::SurfaceId box1_box2 =
        ind.surface_closest_to_point(Point(0.5, 0.5, 1));
    Plc3::SurfaceId box2_box3 =
        ind.surface_closest_to_point(Point(0.5, 0.5, 2));
    EXPECT_EQ(std::min(box1, out), plc.surfaces[box1_out].volumes[0]);
    EXPECT_EQ(std::max(box1, out), plc.surfaces[box1_out].volumes[1]);
    EXPECT_EQ(std::min(box2, out), plc.surfaces[box2_out].volumes[0]);
    EXPECT_EQ(std::max(box2, out), plc.surfaces[box2_out].volumes[1]);
    EXPECT_EQ(std::min(box3, out), plc.surfaces[box3_out].volumes[0]);
    EXPECT_EQ(std::max(box3, out), plc.surfaces[box3_out].volumes[1]);
    EXPECT_EQ(std::min(box1, box2), plc.surfaces[box1_box2].volumes[0]);
    EXPECT_EQ(std::max(box1, box2), plc.surfaces[box1_box2].volumes[1]);
    EXPECT_EQ(std::min(box2, box3), plc.surfaces[box2_box3].volumes[0]);
    EXPECT_EQ(std::max(box2, box3), plc.surfaces[box2_box3].volumes[1]);
}

} /* namespace os2cx */

