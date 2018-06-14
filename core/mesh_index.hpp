#ifndef OS2CX_MESH_INDEX_HPP_
#define OS2CX_MESH_INDEX_HPP_

#include "mesh.hpp"
#include "mesh_type_info.hpp"

namespace os2cx {

class Mesh3Index {
public:
    Mesh3Index(const Mesh3 *mesh);

    /* If face 'face' of element 'el' is directly face-to-face with some face of
    another element, returns the other element and which face. Otherwise,
    returns FaceId::invalid(). */
    FaceId matching_face(FaceId face) const {
        assert(mesh->elements.key_in_range(face.element_id));
        return matching_faces[face];
    }

    std::vector<FaceId> unmatched_faces;

private:
    const Mesh3 *mesh;
    ContiguousMap<FaceId, FaceId> matching_faces;
};

} /* namespace os2cx */

#endif
