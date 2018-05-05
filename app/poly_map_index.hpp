#ifndef OS2CX_POLY_MAP_INDEX_HPP_
#define OS2CX_POLY_MAP_INDEX_HPP_

#include "calc.hpp"
#include "poly_map.hpp"

namespace os2cx {

class Poly3MapIndexInternal;

class Poly3MapIndex {
public:
    Poly3MapIndex(const Poly3Map &pm);
    ~Poly3MapIndex();

    Length approx_scale() const;
    Poly3Map::VolumeId volume_containing_point(Point p) const;
    Poly3Map::SurfaceId surface_closest_to_point(Point p) const;

    const Poly3Map *poly3_map;
    std::unique_ptr<Poly3MapIndexInternal> i;
};

} /* namespace os2cx */

#endif
