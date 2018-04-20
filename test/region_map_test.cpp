#include <assert.h>

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

    assert(rm.vertices.size() == 16);
    assert(rm.volumes.size() == 4);
    assert(rm.surfaces.size() == 5);
    assert(rm.borders.size() == 2);

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
    assert(out == out2);
    assert(out == rm.volume_outside);
    assert(box1 != box3);
    assert(!rm.volumes[out].is_solid);
    assert(rm.volumes[box1].masks[&mask] == false);
    assert(rm.volumes[box2].masks[&mask] == true);
    assert(rm.volumes[box3].masks[&mask] == false);

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
    assert(rm.surfaces[box1_out].volumes[0] == std::min(box1, out));
    assert(rm.surfaces[box1_out].volumes[1] == std::max(box1, out));
    assert(rm.surfaces[box2_out].volumes[0] == std::min(box2, out));
    assert(rm.surfaces[box2_out].volumes[1] == std::max(box2, out));
    assert(rm.surfaces[box3_out].volumes[0] == std::min(box3, out));
    assert(rm.surfaces[box3_out].volumes[1] == std::max(box3, out));
    assert(rm.surfaces[box1_box2].volumes[0] == std::min(box1, box2));
    assert(rm.surfaces[box1_box2].volumes[1] == std::max(box1, box2));
    assert(rm.surfaces[box2_box3].volumes[0] == std::min(box2, box3));
    assert(rm.surfaces[box2_box3].volumes[1] == std::max(box2, box3));
}

} /* namespace os2cx */

