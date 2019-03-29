#ifndef OS2CX_MESH_SPATIAL_INDEX_HPP_
#define OS2CX_MESH_SPATIAL_INDEX_HPP_

#include "attrs.hpp"

namespace os2cx {

class Mesh3SpatialIndexInternal;

class Mesh3SpatialIndex {
public:
    Mesh3SpatialIndex(const Mesh3 *mesh, const FaceSet &face_set);
    ~Mesh3SpatialIndex();

    bool nearest_face(
        Point query,
        double max_dist,
        FaceId *face_out,
        double *signed_dist_out,
        ) const;

private:
    const Mesh3 *mesh;
    std::unique_ptr<Mesh3SpatialIndexInternal> i;
};

} /* namespace os2cx */

#endif
