#include <fstream>

#include <gtest/gtest.h>

#include "plc.hpp"
#include "plc_index.hpp"
#include "plc_nef.hpp"
#include "plc_nef_to_plc.hpp"

namespace os2cx {

TEST(PlcTest, PlcNefToPlc) {
    AttrBitset attrs_solid, attrs_mask;
    attrs_solid.set(0);
    attrs_mask.set(1);
    PlcNef3 solid = PlcNef3::from_poly(Poly3::from_box(Box(0, 0, 0, 1, 1, 3)));
    PlcNef3 mask = PlcNef3::from_poly(Poly3::from_box(Box(-1, -1, 1, 2, 2, 2)));
    solid.binarize(attrs_solid, AttrBitset());
    mask.binarize(attrs_mask, AttrBitset());
    PlcNef3 plc_nef = solid.binary_or(mask);
    plc_nef.map_everywhere([&](AttrBitset bs, PlcNef3::FeatureType) {
        if (bs == attrs_mask) return AttrBitset();
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
    EXPECT_EQ(AttrBitset(), plc.volumes[out].attrs);
    EXPECT_EQ(attrs_solid, plc.volumes[box1].attrs);
    EXPECT_EQ(attrs_solid | attrs_mask, plc.volumes[box2].attrs);
    EXPECT_EQ(attrs_solid, plc.volumes[box3].attrs);

    Plc3::SurfaceId box1_out =
        ind.surface_containing_point(Point(0, 0, 0.5));
    Plc3::SurfaceId box2_out =
        ind.surface_containing_point(Point(0, 0, 1.5));
    Plc3::SurfaceId box3_out =
        ind.surface_containing_point(Point(0, 0, 2.5));
    Plc3::SurfaceId box1_box2 =
        ind.surface_containing_point(Point(0.5, 0.5, 1));
    Plc3::SurfaceId box2_box3 =
        ind.surface_containing_point(Point(0.5, 0.5, 2));
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

