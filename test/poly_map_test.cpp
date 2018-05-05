#include <fstream>

#include <gtest/gtest.h>

#include "poly_map.hpp"
#include "poly_map_index.hpp"

namespace os2cx {

TEST(PolyMapTest, Poly3MapCreate) {
    Poly3 solid = Poly3::box(0, 0, 0, 1, 1, 3);
    Poly3 mask = Poly3::box(-1, -1, 1, 2, 2, 2);
    std::vector<const Poly3 *> masks;
    masks.push_back(&mask);

    Poly3Map pm;
    poly3_map_create(solid, masks, &pm);

    if (false) {
        pm.debug(std::cout);
    }

    ASSERT_EQ(16, pm.vertices.size());
    ASSERT_EQ(4, pm.volumes.size());
    ASSERT_EQ(5, pm.surfaces.size());
    ASSERT_EQ(2, pm.borders.size());

    Poly3MapIndex ind(pm);

    Poly3Map::VolumeId out =
        ind.volume_containing_point(Point::raw(0.5, 0.5, -0.5));
    Poly3Map::VolumeId box1 =
        ind.volume_containing_point(Point::raw(0.5, 0.5, 0.5));
    Poly3Map::VolumeId box2 =
        ind.volume_containing_point(Point::raw(0.5, 0.5, 1.5));
    Poly3Map::VolumeId box3 =
        ind.volume_containing_point(Point::raw(0.5, 0.5, 2.5));
    Poly3Map::VolumeId out2 =
        ind.volume_containing_point(Point::raw(0.5, 0.5, 3.5));
    EXPECT_EQ(out, out2);
    EXPECT_EQ(pm.volume_outside, out);
    EXPECT_NE(box1, box3);
    EXPECT_FALSE(pm.volumes[out].is_solid);
    EXPECT_FALSE(pm.volumes[box1].masks[&mask]);
    EXPECT_TRUE(pm.volumes[box2].masks[&mask]);
    EXPECT_FALSE(pm.volumes[box3].masks[&mask]);

    Poly3Map::SurfaceId box1_out =
        ind.surface_closest_to_point(Point::raw(0, 0, 0.5));
    Poly3Map::SurfaceId box2_out =
        ind.surface_closest_to_point(Point::raw(0, 0, 1.5));
    Poly3Map::SurfaceId box3_out =
        ind.surface_closest_to_point(Point::raw(0, 0, 2.5));
    Poly3Map::SurfaceId box1_box2 =
        ind.surface_closest_to_point(Point::raw(0.5, 0.5, 1));
    Poly3Map::SurfaceId box2_box3 =
        ind.surface_closest_to_point(Point::raw(0.5, 0.5, 2));
    EXPECT_EQ(std::min(box1, out), pm.surfaces[box1_out].volumes[0]);
    EXPECT_EQ(std::max(box1, out), pm.surfaces[box1_out].volumes[1]);
    EXPECT_EQ(std::min(box2, out), pm.surfaces[box2_out].volumes[0]);
    EXPECT_EQ(std::max(box2, out), pm.surfaces[box2_out].volumes[1]);
    EXPECT_EQ(std::min(box3, out), pm.surfaces[box3_out].volumes[0]);
    EXPECT_EQ(std::max(box3, out), pm.surfaces[box3_out].volumes[1]);
    EXPECT_EQ(std::min(box1, box2), pm.surfaces[box1_box2].volumes[0]);
    EXPECT_EQ(std::max(box1, box2), pm.surfaces[box1_box2].volumes[1]);
    EXPECT_EQ(std::min(box2, box3), pm.surfaces[box2_box3].volumes[0]);
    EXPECT_EQ(std::max(box2, box3), pm.surfaces[box2_box3].volumes[1]);
}

} /* namespace os2cx */

