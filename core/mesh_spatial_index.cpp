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

    template<class Iterator>
    Mesh3AabbPrimitive(Iterator it, const Mesh3 *) {
        *this = *it;
    }

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

    FaceId face_id;
    NodeId nodes[3];

private:
    Point get_point(const Mesh3 *mesh, NodeId node_id) const {
        return CGAL::Point_3<KS>(
            mesh->nodes[node_id].point.x,
            mesh->nodes[node_id].point.y,
            mesh->nodes[node_id].point.z);
    }
};

typedef CGAL::AABB_traits<KS, Mesh3AabbPrimitive> Mesh3AabbTraits;
typedef CGAL::AABB_tree<Mesh3AabbTraits> Mesh3AabbTree;

class Mesh3SpatialIndexInternal {
public:
    Mesh3AabbTree tree;
};

/* Iterator yielding Mesh3AabbPrimitive, generated from an inner iterator
yielding FaceId. */
template<class FaceIdIterator>
class Mesh3AabbPrimitiveIterator {
public:
    bool operator!=(const Mesh3AabbPrimitiveIterator &other) const {
        return mesh != other.mesh
            || face_it != other.face_it
            || triangle != other.triangle;
    }
    Mesh3AabbPrimitiveIterator &operator++() {
        ++triangle;
        if (triangle == num_triangles_for_face(*face_it)) {
            ++face_it;
            triangle = 0;
        }
        return *this;
    }
    Mesh3AabbPrimitiveIterator operator++(int) {
        Mesh3AabbPrimitiveIterator copy = *this;
        ++*this;
        return copy;
    }
    Mesh3AabbPrimitive operator*() const {
        Mesh3AabbPrimitive primitive;
        primitive.face_id = *face_it;
        const Element3 &element = mesh->elements[face_it->element_id];
        const ElementTypeShape &shape = element_type_shape(element.type);
        const ElementTypeShape::Face &face = shape.faces.at(face_it->face);
        int subtriangle = it->second;
        int indices[3];
        if (face.vertices.size() == 3) {
            indices[0] = 0;
            indices[1] = 1;
            indices[2] = 2;
        } else if (face.vertices.size() == 4) {
            indices[0] = 0;
            indices[1] = 1 + triangle;
            indices[2] = 2 + triangle;
        } else if (face.vertices.size() == 6) {
            indices[0] = 0;
            indices[1] = 2;
            indices[2] = 4;
        } else if (face.vertices.size() == 8) {
            indices[0] = 0;
            indices[1] = 2 + 2 * triangle;
            indices[2] = 4 + 2 * triangle;
        }
        for (int i = 0; i < 3; ++i) {
            primitive.nodes[i] = element.nodes[face.vertices[indices[i]]];
        }
    }
    Mesh3AabbPrimitive operator->() const {
        return **this;
    }

    int num_triangles_for_face(FaceId face_id) const {
        const Element3 &element = mesh->elements[face_id.element_id];
        const ElementTypeShape &shape = element_type_shape(element.type);
        const ElementTypeShape::Face &face = shape.faces.at(face_id.face);
        if (face.vertices.size() == 3 || face.vertices.size() == 6) {
            return 1;
        } else {
            return 2;
        }
    }

    const Mesh3 *mesh;
    FaceIdIterator face_it;
    int triangle;
};

/* Iterator yielding FaceId, generated from an inner iterator yielding
ElementId. */
template<class ElementIdIterator>
class Mesh3FaceIdIterator {
public:
    bool operator!=(const Mesh3FaceIdIterator &other) const {
        return mesh != other.mesh
            || element_it != other.element_id
            || face != other.face;
    }
    Mesh3FaceIdIterator &operator++() {
        ++face;
        if (face == mesh->elements[*element_it].num_faces()) {
            ++element_it;
            face = 0;
        }
        return *this;
    }
    Mesh3FaceIdIterator operator++(int) {
        Mesh3FaceIdIterator copy = *this;
        ++*this;
        return copy;
    }
    FaceId operator*() const {
        return FaceId(*element_it, face);
    }
    FaceId operator->() const {
        return **this;
    }

    const Mesh3 *mesh;
    ElementIdIterator element_it;
    int face;
};

/* Iterator yielding ElementId in a contiguous range. */
class Mesh3ElementIdIterator {
public:
    bool operator!=(const Mesh3ElementIdIterator &other) const {
        return element_id != other.element_id;
    }
    Mesh3ElementIdIterator &operator++() {
        ++element_id;
        return *this;
    }
    Mesh3ElementIdIterator operator++(int) {
        Mesh3ElementIdIterator copy = *this;
        ++*this;
        return copy;
    }
    ElementId operator*() const {
        return element_id;
    }

private:
    ElementId element_id;
};

Mesh3SpatialIndex::Mesh3SpatialIndex(const Mesh3 *mesh) {
    Mesh3AabbPrimitiveIterator<Mesh3FaceIdIterator<Mesh3ElementIdIterator> >
        begin, end;
    begin.mesh = mesh;
    begin.face_it.mesh = mesh;
    begin.face_it.element_it.element_id = mesh->elements.key_begin();
    begin.face_it.face = 0;
    begin.triangle = 0;
    end.mesh = mesh;
    end.face_it.mesh = mesh;
    end.face_it.element_it.element_id = mesh->elements.key_end();
    end.face_it.face = 0;
    end.triangle = 0;
    i.reset(new Mesh3SpatialIndexInternal);
    i->tree.rebuild(begin, end, mesh);
}

Mesh3SpatialIndex::Mesh3SpatialIndex(
    const Mesh3 *mesh,
    const ElementSet &element_set
) {
    Mesh3AabbPrimitiveIterator<Mesh3FaceIdIterator<
        std::set<ElementId>::const_iterator> > begin, end;
    begin.mesh = mesh;
    begin.face_it.mesh = mesh;
    begin.face_it.element_it = element_set.elements.begin();
    begin.face_it.face = 0;
    begin.triangle = 0;
    end.mesh = mesh;
    end.face_it.mesh = mesh;
    end.face_it.element_it = element_set.elements.end();
    end.face_it.face = 0;
    end.triangle = 0;
    i.reset(new Mesh3SpatialIndexInternal);
    i->tree.rebuild(begin, end, mesh);
}

Mesh3SpatialIndex::Mesh3SpatialIndex(
    const Mesh3 *mesh,
    const FaceSet &face_set
) {
    Mesh3AabbPrimitiveIterator<std::set<FaceId>::const_iterator> begin, end;
    begin.mesh = mesh;
    begin.face_it = face_set.faces.begin();
    begin.triangle = 0;
    end.mesh = mesh;
    end.face_it = face_set.faces.end();
    end.triangle = 0;
    i.reset(new Mesh3SpatialIndexInternal);
    i->tree.rebuild(begin, end, mesh);
}

Mesh3SpatialIndex::~Mesh3SpatialIndex() { }

} /* namespace os2cx */
