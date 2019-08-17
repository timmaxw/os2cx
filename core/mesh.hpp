#ifndef OS2CX_MESH_HPP_
#define OS2CX_MESH_HPP_

#include <assert.h>

#include <set>
#include <string>
#include <vector>

#include "calc.hpp"
#include "mesh_type_info.hpp"
#include "util.hpp"

namespace os2cx {

class Node3 {
public:
    Point point;
};

class NodeId {
public:
    static NodeId from_int(int id) { NodeId ni; ni.id = id; return ni; }
    static NodeId invalid() { return NodeId::from_int(-1); }
    int to_int() const { return id; }
    bool operator==(NodeId other) const { return id == other.id; }
    bool operator!=(NodeId other) const { return id != other.id; }
    bool operator<(NodeId other) const { return id < other.id; }
    void operator++() { ++id; }
private:
    int id;
};

class Element3 {
public:
    int num_nodes() const {
        return element_type_shape(type).vertices.size();
    }

    ElementType type;
    NodeId nodes[ElementTypeShape::max_vertices_per_element];
};

class ElementId {
public:
    static ElementId from_int(int id) { ElementId ei; ei.id = id; return ei; }
    static ElementId invalid() { return ElementId::from_int(-1); }
    int to_int() const { return id; }
    bool operator==(ElementId other) const { return id == other.id; }
    bool operator!=(ElementId other) const { return id != other.id; }
    bool operator<(ElementId other) const { return id < other.id; }
    void operator++() { ++id; }
private:
    int id;
};

class FaceId {
public:
    static FaceId from_int(int id) {
        FaceId fi;
        fi.element_id = ElementId::from_int(
            id / ElementTypeShape::max_faces_per_element);
        fi.face = id % ElementTypeShape::max_faces_per_element;
        return fi;
    }
    static FaceId invalid() {
        FaceId fi;
        fi.element_id = ElementId::invalid();
        fi.face = 0;
        return fi;
    }
    FaceId() { }
    FaceId(ElementId ei, int f) : element_id(ei), face(f) { }
    int to_int() {
        return element_id.to_int() * ElementTypeShape::max_faces_per_element
            + face;
    }
    bool operator==(FaceId other) const {
        return element_id == other.element_id && face == other.face;
    }
    bool operator!=(FaceId other) const {
        return !(*this == other);
    }
    bool operator<(FaceId other) const {
        return (element_id < other.element_id) ||
            (element_id == other.element_id && face < other.face);
    }
    ElementId element_id;
    int face;
};

/* Mesh3 is a collection of 3D nodes and elements suitable for use in a
simulation.

Not to be confused with Poly3; a Poly3 only defines a surface, and uses
triangles of whatever size is most convenient, whereas a Mesh3 fills a volume
with tetrahedra or other elements, and subdivides them to a size that's good for
simulation. */

class Mesh3 {
public:
    Mesh3() :
        nodes(NodeId::from_int(1)),
        elements(ElementId::from_int(1))
        { }

    void append_mesh(
        const Mesh3 &other,
        NodeId *new_node_begin_out,
        NodeId *new_node_end_out,
        ElementId *new_element_begin_out,
        ElementId *new_element_end_out);

    /* Computes the volume of the given element. */
    Volume volume(const Element3 &element) const;

    /* Computes the weighted "volume" influenced by each node of the given
    element. This is useful for e.g. converting a force distributed uniformly
    over the volume of the element into an equivalent set of forces on the nodes
    of the element. */
    void volumes_for_nodes(const Element3 &element, Volume *volumes_out) const;

    /* Computes the oriented area of the given face of the given element. (For
    linear elements, this can be thought of as a vector pointing perpendicularly
    out from the face with magnitude equal to the face's area; although this
    description isn't quite right for a quadratic element with a curved face.)
    */
    Vector oriented_area(const Element3 &element, int face_index) const;

    /* Computes the weighted "oriented areas" influenced by each node of the
    given element. This is useful for e.g. converting a force distributed
    uniformly over the area of the face into an equivalent set of forces on the
    nodes. Nodes that aren't part of the face will have values set to zero. */
    void oriented_areas_for_nodes(
        const Element3 &element,
        int face_index,
        Vector *areas_out
    ) const;

    /* Computes the center of mass and volume of the given element. */
    void center_of_mass(
        const Element3 &element,
        Point *center_of_mass_out,
        Volume *volume_out
    ) const;

    ContiguousMap<NodeId, Node3> nodes;
    ContiguousMap<ElementId, Element3> elements;

private:
    Point point_for_shape_point(
        const Element3 &element,
        ElementTypeShape::ShapePoint uvw
    ) const;

    Matrix jacobian(
        const Element3 &element,
        ElementTypeShape::ShapePoint uvw
    ) const;

    double integrate_volume(
        const Element3 &element,
        const ElementTypeShape::IntegrationPoint &ip
    ) const;

    Vector integrate_area(
        const Element3 &element,
        int face_index,
        const ElementTypeShape::IntegrationPoint &ip
    ) const;

};

} /* namespace os2cx */

#endif
