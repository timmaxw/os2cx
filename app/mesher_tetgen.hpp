#ifndef OS2CX_MESHER_TETGEN_HPP_
#define OS2CX_MESHER_TETGEN_HPP_

#include "mesh.hpp"
#include "region_map.hpp"

namespace os2cx {

Mesh3 mesher_tetgen(
    const RegionMap3 &region_map);

} /* namespace os2cx */

#endif
