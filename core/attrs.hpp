#ifndef OS2CX_ATTRS_HPP_
#define OS2CX_ATTRS_HPP_

#include "mesh.hpp"
#include "mesh_index.hpp"
#include "plc.hpp"
#include "plc_nef.hpp"
#include "plc_index.hpp"

namespace os2cx {

Plc3::BitIndex bit_index_solid();

PlcNef3 compute_plc_nef_for_solid(const Poly3 &solid);

void compute_plc_nef_select_volume(
    PlcNef3 *solid_nef,
    const Poly3 &mask,
    Plc3::BitIndex bit_index_mask);

void compute_plc_nef_select_surface(
    PlcNef3 *solid_nef,
    const Poly3 &mask,
    Vector direction_vector,
    double direction_angle_tolerance,
    Plc3::BitIndex bit_index_mask);

class ElementSet {
public:
    std::set<ElementId> elements;
};

ElementSet compute_element_set_from_range(ElementId begin, ElementId end);

ElementSet compute_element_set_from_plc_bit(
    const Plc3Index &plc_index,
    const Mesh3 &mesh,
    ElementId element_begin,
    ElementId element_end,
    Plc3::BitIndex bit_index);

class FaceSet {
public:
    std::set<FaceId> faces;
};

FaceSet compute_face_set_from_plc_bit(
    const Plc3Index &plc_index,
    const Mesh3 &mesh,
    const Mesh3Index &mesh_index,
    ElementId element_begin,
    ElementId element_end,
    Plc3::BitIndex bit_index);

class NodeSet {
public:
    std::set<NodeId> nodes;
};

NodeSet compute_node_set_from_range(NodeId begin, NodeId end);

NodeSet compute_node_set_from_element_set(
    const Mesh3 &mesh,
    const ElementSet &element_set);

NodeSet compute_node_set_from_face_set(
    const Mesh3 &mesh,
    const FaceSet &face_set);

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
    Vector force_density);

} /* namespace os2cx */

#endif

