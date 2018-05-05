#include <fstream>

#include <gtest/gtest.h>

#include "region_map.hpp"
#include "region_map_index.hpp"

namespace os2cx {

TEST(RegionMapTest, RegionMapCreate) {
    Region3 solid = Region3::box(0, 0, 0, 1, 1, 3);
    Region3 mask = Region3::box(-1, -1, 1, 2, 2, 2);
    std::vector<const Region3 *> masks;
    masks.push_back(&mask);

    RegionMap3 rm;
    region_map_create(solid, masks, &rm);

    if (false) {
        rm.debug(std::cout);
    }

    ASSERT_EQ(16, rm.vertices.size());
    ASSERT_EQ(4, rm.volumes.size());
    ASSERT_EQ(5, rm.surfaces.size());
    ASSERT_EQ(2, rm.borders.size());

    RegionMap3Index ind(rm);

    RegionMap3::VolumeId out =
        ind.volume_containing_point(Point::raw(0.5, 0.5, -0.5));
    RegionMap3::VolumeId box1 =
        ind.volume_containing_point(Point::raw(0.5, 0.5, 0.5));
    RegionMap3::VolumeId box2 =
        ind.volume_containing_point(Point::raw(0.5, 0.5, 1.5));
    RegionMap3::VolumeId box3 =
        ind.volume_containing_point(Point::raw(0.5, 0.5, 2.5));
    RegionMap3::VolumeId out2 =
        ind.volume_containing_point(Point::raw(0.5, 0.5, 3.5));
    EXPECT_EQ(out, out2);
    EXPECT_EQ(rm.volume_outside, out);
    EXPECT_NE(box1, box3);
    EXPECT_FALSE(rm.volumes[out].is_solid);
    EXPECT_FALSE(rm.volumes[box1].masks[&mask]);
    EXPECT_TRUE(rm.volumes[box2].masks[&mask]);
    EXPECT_FALSE(rm.volumes[box3].masks[&mask]);

    RegionMap3::SurfaceId box1_out =
        ind.surface_closest_to_point(Point::raw(0, 0, 0.5));
    RegionMap3::SurfaceId box2_out =
        ind.surface_closest_to_point(Point::raw(0, 0, 1.5));
    RegionMap3::SurfaceId box3_out =
        ind.surface_closest_to_point(Point::raw(0, 0, 2.5));
    RegionMap3::SurfaceId box1_box2 =
        ind.surface_closest_to_point(Point::raw(0.5, 0.5, 1));
    RegionMap3::SurfaceId box2_box3 =
        ind.surface_closest_to_point(Point::raw(0.5, 0.5, 2));
    EXPECT_EQ(std::min(box1, out), rm.surfaces[box1_out].volumes[0]);
    EXPECT_EQ(std::max(box1, out), rm.surfaces[box1_out].volumes[1]);
    EXPECT_EQ(std::min(box2, out), rm.surfaces[box2_out].volumes[0]);
    EXPECT_EQ(std::max(box2, out), rm.surfaces[box2_out].volumes[1]);
    EXPECT_EQ(std::min(box3, out), rm.surfaces[box3_out].volumes[0]);
    EXPECT_EQ(std::max(box3, out), rm.surfaces[box3_out].volumes[1]);
    EXPECT_EQ(std::min(box1, box2), rm.surfaces[box1_box2].volumes[0]);
    EXPECT_EQ(std::max(box1, box2), rm.surfaces[box1_box2].volumes[1]);
    EXPECT_EQ(std::min(box2, box3), rm.surfaces[box2_box3].volumes[0]);
    EXPECT_EQ(std::max(box2, box3), rm.surfaces[box2_box3].volumes[1]);
}

} /* namespace os2cx */

