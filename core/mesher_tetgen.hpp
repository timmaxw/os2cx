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
    MaxElementSize max_element_size_default,
    const AttrOverrides<MaxElementSize> &max_element_size_overrides,
    ElementType element_type);

} /* namespace os2cx */

#endif
