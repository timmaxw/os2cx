#ifndef OS2CX_MESHER_TETGEN_HPP_
#define OS2CX_MESHER_TETGEN_HPP_

#include "mesh.hpp"
#include "plc.hpp"

namespace os2cx {

Mesh3 mesher_tetgen(
    const Plc3 &plc,
    double max_tet_volume);

double suggest_max_tet_volume(const Plc3 &plc);

} /* namespace os2cx */

#endif
