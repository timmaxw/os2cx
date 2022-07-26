#ifndef OS2CX_COMPUTE_ATTRS_HPP_
#define OS2CX_COMPUTE_ATTRS_HPP_

#include <map>

#include "mesh.hpp"
#include "mesh_index.hpp"
#include "plc.hpp"
#include "plc_nef.hpp"
#include "plc_index.hpp"

namespace os2cx {

PlcNef3 compute_plc_nef_for_solid(const Poly3 &solid);

void compute_plc_nef_select_volume(
    PlcNef3 *solid_nef,
    const Poly3 &mask,
    AttrBitIndex attr_bit_mask);

void compute_plc_nef_select_surface_external(
    PlcNef3 *solid_nef,
    const Poly3 &mask,
    Vector direction_vector,
    double direction_angle_tolerance,
    AttrBitIndex attr_bit_mask);

void compute_plc_nef_select_surface_internal(
    PlcNef3 *solid_nef,
    const Poly3 &mask,
    Vector direction_vector,
    double direction_angle_tolerance,
    AttrBitIndex attr_bit_mask);

void compute_plc_nef_select_node(
    PlcNef3 *solid_nef,
    Point point,
    AttrBitIndex attr_bit_mask);

MaxElementSize suggest_max_element_size(const Plc3 &plc);

class ElementSet {
public:
    std::set<ElementId> elements;
};

ElementSet compute_element_set_from_range(ElementId begin, ElementId end);

ElementSet compute_element_set_from_attr_bit(
    const Mesh3 &mesh,
    ElementId element_begin,
    ElementId element_end,
    AttrBitIndex attr_bit);

class FaceSet {
public:
    std::set<FaceId> faces;
};

FaceSet compute_face_set_from_attr_bit(
    const Mesh3 &mesh,
    ElementId element_begin,
    ElementId element_end,
    Vector direction_vector,
    double direction_angle_tolerance,
    AttrBitIndex attr_bit);

class NodeSet {
public:
    std::set<NodeId> nodes;
};

NodeSet compute_node_set_singleton(NodeId node);

NodeSet compute_node_set_from_range(NodeId begin, NodeId end);

NodeSet compute_node_set_from_element_set(
    const Mesh3 &mesh,
    const ElementSet &element_set);

NodeSet compute_node_set_from_face_set(
    const Mesh3 &mesh,
    const FaceSet &face_set);

NodeId compute_node_id_from_attr_bit(
    const Mesh3 &mesh,
    NodeId node_begin,
    NodeId node_end,
    AttrBitIndex attr_bit);

class ConcentratedLoad {
public:
    class Load {
    public:
        Load() : force(0, 0, 0) { }
        Vector force;
    };
    std::map<NodeId, Load> loads;
};

ConcentratedLoad compute_load_from_element_set(
    const Mesh3 &mesh,
    const ElementSet &element_set,
    Vector force_total_or_per_volume,
    bool force_is_per_volume);

ConcentratedLoad compute_load_from_face_set(
    const Mesh3 &mesh,
    const FaceSet &face_set,
    Vector force_total_or_per_area,
    bool force_is_per_area);

class Slice {
public:
    class Pair {
    public:
        NodeId nodes[2];
        Vector normal; // unit vector
    };

    void append_slice(const Slice &other, const MeshIdMapping &id_mapping);

    std::vector<Pair> pairs;
};

Slice compute_slice(
    /* Warning: Mutates 'mesh'! Any node IDs, element IDs, and/or Mesh3Index on
    'mesh' will be invalidated. */
    Mesh3 *mesh,
    const FaceSet &face_set);

class LinearEquation {
public:
    class Variable {
    public:
        Variable(NodeId n, Dimension d) : node_id(n), dimension(d) { }
        NodeId node_id;
        Dimension dimension;

        bool operator==(Variable v) const {
            return (node_id == v.node_id && dimension == v.dimension);
        }
        bool operator<(Variable v) const {
            return (node_id < v.node_id)
                || (node_id == v.node_id && dimension < v.dimension);
        }
    };
    std::map<Variable, double> terms;
};

std::vector<LinearEquation> compute_equations_for_slice(const Slice &slice);

} /* namespace os2cx */

#endif

