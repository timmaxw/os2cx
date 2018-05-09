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
        return ElementTypeInfo::get(type).shape->vertices.size();
    }

    ElementType type;
    NodeId nodes[20];
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
            id / ElementShapeInfo::max_faces_per_element);
        fi.face = id % ElementShapeInfo::max_faces_per_element;
        return fi;
    }
    static FaceId invalid() {
        FaceId fi;
        fi.element_id = ElementId::invalid();
        fi.face = 0;
        return fi;
    }
    int to_int() {
        return element_id.to_int() * ElementShapeInfo::max_faces_per_element
            + face;
    }
    bool operator==(FaceId other) const {
        return element_id == other.element_id && face == other.face;
    }
    bool operator!=(FaceId other) const {
        return !(*this == other);
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

    Volume volume(const Element3 &element) const;
    Volume volume_for_node(const Element3 &element, int which) const;

    ContiguousMap<NodeId, Node3> nodes;
    ContiguousMap<ElementId, Element3> elements;

private:
    /* Plugs the coordinates of the element's vertices into the given
    polynomial, whose variables are all of the form
    shape_function_variables::coord. */
    double evaluate_polynomial(
        const Element3 &element,
        const Polynomial &poly) const;
};

} /* namespace os2cx */

#endif
