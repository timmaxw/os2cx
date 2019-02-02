#ifndef OS2CX_MESHER_NAIVE_BRICKS_HPP_
#define OS2CX_MESHER_NAIVE_BRICKS_HPP_

#include "mesh.hpp"
#include "plc.hpp"

namespace os2cx {

Mesh3 mesher_naive_bricks(
    const Plc3 &plc,
    double max_tet_volume);

} /* namespace os2cx */

#endif
