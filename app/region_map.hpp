#ifndef OS2CX_REGION_MAP_HPP_
#define OS2CX_REGION_MAP_HPP_

#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "calc.hpp"
#include "region.hpp"

namespace os2cx {

class RegionMap3Internal;

class RegionMap3 {
public:
    typedef int VertexId;
    class Vertex {
    public:
        Point point;
    };
    std::vector<Vertex> vertices;

    typedef int VolumeId;
    class Volume {
    public:
        bool is_solid;
        std::map<const Region3 *, bool> masks;
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
            PureVector normal;
        };
        std::vector<Triangle> triangles;
        VolumeId volumes[2];
        // std::map<const Region3 *, bool> masks;
    };
    std::vector<Surface> surfaces;

    typedef int BorderId;
    class Border {
    public:
        std::deque<VertexId> vertices;
        std::set<SurfaceId> surfaces;
    };
    std::vector<Border> borders;
    std::map<std::pair<VertexId, VertexId>, BorderId> borders_by_vertex;

    RegionMap3();
    ~RegionMap3();

    void debug(std::ostream &stream) const;

    std::unique_ptr<RegionMap3Internal> i;
};

void region_map_create(
    const Region3 &solid,
    const std::vector<const Region3 *> &masks,
    RegionMap3 *region_map_out);

} /* namespace os2cx */

#endif /* OS2CX_REGION_MAP_HPP_ */

