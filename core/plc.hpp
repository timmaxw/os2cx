#ifndef OS2CX_PLC_HPP_
#define OS2CX_PLC_HPP_

#include <bitset>
#include <deque>
#include <vector>
#include <set>

#include "calc.hpp"

namespace os2cx {

class Plc3
{
public:
    static const int num_bits = 64;
    typedef std::bitset<num_bits> Bitset;

    typedef int VertexId;
    class Vertex {
    public:
        Point point;
        Bitset bitset;
    };
    std::vector<Vertex> vertices;

    typedef int VolumeId;
    class Volume {
    public:
        Bitset bitset;
    };
    std::vector<Volume> volumes;
    VolumeId volume_outside;

    typedef int SurfaceId;
    class Surface {
    public:
        /* By convention, all triangles' vertices are ordered counterclockwise
        when looking from volumes[0] into volumes[1], and all normal vectors
        point into volumes[0]. */
        class Triangle {
        public:
            VertexId vertices[3];
        };
        std::vector<Triangle> triangles;
        VolumeId volumes[2];
        Bitset bitset;
    };
    std::vector<Surface> surfaces;

    typedef int BorderId;
    class Border {
    public:
        std::deque<VertexId> vertices;
        std::vector<SurfaceId> surfaces;
        Bitset bitset;
    };
    std::vector<Border> borders;

    void debug(std::ostream &) const;
};

} /* namespace os2cx */

#endif /* OS2CX_PLC_HPP_ */
