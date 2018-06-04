#ifndef OS2CX_POLY_MAP_HPP_
#define OS2CX_POLY_MAP_HPP_

#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "calc.hpp"
#include "poly.hpp"

namespace os2cx {

class Poly3MapInternal;

class Poly3Map {
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

    Poly3Map();
    ~Poly3Map();

    void debug(std::ostream &stream) const;

    std::unique_ptr<Poly3MapInternal> i;
};

class Poly3MapVolumeMask {
public:
    const Poly3 *poly;
};

void poly3_map_create(
    const Poly3 &solid,
    const std::vector<Poly3MapVolumeMask> &volume_masks,
    Poly3Map *poly3_map_out,
    const std::vector<std::set<Poly3Map::VolumeId> *>
        &volume_mask_volumes_out);

} /* namespace os2cx */

#endif

