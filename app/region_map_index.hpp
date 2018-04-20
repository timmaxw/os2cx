#ifndef OS2CX_REGION_MAP_INDEX_HPP_
#define OS2CX_REGION_MAP_INDEX_HPP_

#include "calc.hpp"
#include "region_map.hpp"

namespace os2cx {

class RegionMap3IndexInternal;

class RegionMap3Index {
public:
    RegionMap3Index(const RegionMap3 &rm);
    ~RegionMap3Index();

    Length approx_scale() const;
    RegionMap3::VolumeId volume_containing_point(Point p) const;
    RegionMap3::SurfaceId surface_closest_to_point(Point p) const;

    const RegionMap3 *region_map;
    std::unique_ptr<RegionMap3IndexInternal> i;
};

} /* namespace os2cx */

#endif
