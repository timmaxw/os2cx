#include "mesh_spatial_index.hpp"

#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/Simple_cartesian.h>

namespace os2cx {

typedef CGAL::Simple_cartesian<double> KS;

/* This is a model of the concept CGAL::AABBPrimitiveWithSharedData */
class Mesh3AabbPrimitive {
public:
    typedef CGAL::Point_3<KS> Point;
    typedef CGAL::Triangle_3<KS> Datum;
    typedef FaceId Id;
    typedef const Mesh3 *Shared_data;

    static Shared_data construct_shared_data(const Mesh3 *mesh) {
        return mesh;
    }

    Mesh3AabbPrimitive() { }
    Mesh3AabbPrimitive(FaceId fi, NodeId n1, NodeId n2, NodeId n3) :
        face_id(fi), nodes({n1, n2, n3}) { }

    Datum datum(const Mesh3 *mesh) const {
        return CGAL::Triangle_3<KS>(
            get_point(mesh, nodes[0]),
            get_point(mesh, nodes[1]),
            get_point(mesh, nodes[2]));
    }

    Id id() const {
        return face_id;
    }

    Point reference_point(const Mesh3 *mesh) const {
        return get_point(mesh, nodes[0]);
    }

private:
    Point get_point(const Mesh3 *mesh, NodeId node_id) const {
        return CGAL::Point_3<KS>(
            mesh->nodes[node_id].point.x,
            mesh->nodes[node_id].point.y,
            mesh->nodes[node_id].point.z);
    }

    FaceId face_id;
    NodeId nodes[3];
};

typedef CGAL::AABB_traits<KS, Mesh3AabbPrimitive> Mesh3AabbTraits;
typedef CGAL::AABB_tree<Mesh3AabbTraits> Mesh3AabbTree;

class Mesh3SpatialIndexInternal {
public:
    Mesh3AabbTree tree;
};

Mesh3SpatialIndex::Mesh3SpatialIndex(
    const Mesh3 *_mesh,
    const FaceSet &face_set) :
    mesh(_mesh)
{
    i.reset(new Mesh3SpatialIndexInternal);
    for (FaceId face_id : face_set.faces) {
        const Element3 &element = mesh->elements[face_id.element_id];
        const ElementTypeShape &shape = element_type_shape(element.type);
        const ElementTypeShape::Face face_shape = shape.faces[face_id.face];
        if (face_shape.vertices.size() == 3) {
            i->tree.insert(Mesh3AabbPrimitive(face_id,
                element.nodes[face_shape.vertices[0]],
                element.nodes[face_shape.vertices[1]],
                element.nodes[face_shape.vertices[2]]));
        } else if (face_shape.vertices.size() == 6) {
            i->tree.insert(Mesh3AabbPrimitive(face_id,
                element.nodes[face_shape.vertices[0]],
                element.nodes[face_shape.vertices[2]],
                element.nodes[face_shape.vertices[4]]));
        } else if (face_shape.vertices.size() == 4) {
            i->tree.insert(Mesh3AabbPrimitive(face_id,
                element.nodes[face_shape.vertices[0]],
                element.nodes[face_shape.vertices[1]],
                element.nodes[face_shape.vertices[2]]));
            i->tree.insert(Mesh3AabbPrimitive(face_id,
                element.nodes[face_shape.vertices[0]],
                element.nodes[face_shape.vertices[2]],
                element.nodes[face_shape.vertices[3]]));
        } else if (face_shape.vertices.size() == 8) {
            i->tree.insert(Mesh3AabbPrimitive(face_id,
                element.nodes[face_shape.vertices[0]],
                element.nodes[face_shape.vertices[2]],
                element.nodes[face_shape.vertices[4]]));
            i->tree.insert(Mesh3AabbPrimitive(face_id,
                element.nodes[face_shape.vertices[0]],
                element.nodes[face_shape.vertices[4]],
                element.nodes[face_shape.vertices[6]]));
        }
    }
    i->tree.build();
    i->tree.accelerate_distance_queries();
}

Mesh3SpatialIndex::~Mesh3SpatialIndex() { }

} /* namespace os2cx */
