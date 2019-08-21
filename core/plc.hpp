#ifndef OS2CX_PLC_HPP_
#define OS2CX_PLC_HPP_

#include <deque>
#include <vector>
#include <set>

#include "attrs.hpp"
#include "calc.hpp"

namespace os2cx {

class Plc3
{
public:
    typedef int VertexId;
    class Vertex {
    public:
        Point point;
        AttrBitset attrs;
    };
    std::vector<Vertex> vertices;

    typedef int VolumeId;
    class Volume {
    public:
        AttrBitset attrs;
    };
    std::vector<Volume> volumes;
    VolumeId volume_outside;

    typedef int SurfaceId;
    class Surface {
    public:
        /* By convention, all triangles' vertices are ordered counterclockwise
        when looking from volumes[0] into volumes[1]. */
        class Triangle {
        public:
            VertexId vertices[3];
        };
        std::vector<Triangle> triangles;
        VolumeId volumes[2];
        AttrBitset attrs;
    };
    std::vector<Surface> surfaces;

    typedef int BorderId;
    class Border {
    public:
        std::deque<VertexId> vertices;
        std::vector<SurfaceId> surfaces;
        AttrBitset attrs;
    };
    std::vector<Border> borders;

    void debug(std::ostream &) const;
};

} /* namespace os2cx */

#endif /* OS2CX_PLC_HPP_ */
