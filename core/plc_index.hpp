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

    Length approx_scale() const;
    Plc3::VolumeId volume_containing_point(Point p) const;
    Plc3::SurfaceId surface_closest_to_point(Point p) const;

    const Plc3 *plc;
    std::unique_ptr<Plc3IndexInternal> i;
};

} /* namespace os2cx */

#endif /* OS2CX_PLC_INDEX_HPP_ */
