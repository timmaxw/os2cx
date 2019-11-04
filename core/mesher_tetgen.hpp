#ifndef OS2CX_MESHER_TETGEN_HPP_
#define OS2CX_MESHER_TETGEN_HPP_

#include "mesh.hpp"
#include "plc.hpp"

namespace os2cx {

class TetgenError : public std::runtime_error {
public:
    TetgenError(const char *s) : std::runtime_error(s) { }
};

Mesh3 mesher_tetgen(
    const Plc3 &plc,
    double max_element_size);

void transfer_attrs(const Plc3 &plc, Mesh3 *mesh);

double compute_bbox_volume(const Plc3 &plc);

double suggest_max_element_size(const Plc3 &plc);

} /* namespace os2cx */

#endif
