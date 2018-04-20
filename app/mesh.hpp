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
        return ElementTypeInfo::get(type).shape->nodes.size();
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

class Mesh3 {
public:
    Mesh3() :
        nodes(NodeId::from_int(1)),
        elements(ElementId::from_int(1))
        { }

    Volume volume(const Element3 &element) const;
    Volume volume_for_node(const Element3 &element, int which) const;

    ContiguousMap<NodeId, Node3> nodes;
    ContiguousMap<ElementId, Element3> elements;
};

class MeshIdAllocator {
public:
    MeshIdAllocator() :
        next_node_id(NodeId::from_int(1)),
        next_element_id(ElementId::from_int(1))
        { }
    Mesh3 allocate(Mesh3 &&mesh);

    NodeId next_node_id;
    ElementId next_element_id;
};

} /* namespace os2cx */

#endif
