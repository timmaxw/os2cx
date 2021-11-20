#ifndef OS2CX_PLC_INDEX_HPP_
#define OS2CX_PLC_INDEX_HPP_

#include <memory>

#include "calc.hpp"
#include "plc.hpp"

namespace os2cx {

class Plc3IndexInternal;

class Plc3Index {
public:
    Plc3Index(const Plc3 *plc);
    ~Plc3Index();

    Plc3::VolumeId volume_containing_point(Point p) const;

    /* If the point is on a surface (to within some epsilon), returns that
    surface; otherwise, returns -1. */
    Plc3::SurfaceId surface_containing_point(Point p) const;

    /* If the point is at a vertex (to within some epsilon), returns that
    vertex; otherwise, returns -1. */
    Plc3::VertexId vertex_at_point(Point p) const;

    const Plc3 *plc;
    std::unique_ptr<Plc3IndexInternal> i;
};

} /* namespace os2cx */

#endif /* OS2CX_PLC_INDEX_HPP_ */
