#ifndef OS2CX_ATTRS_HPP_
#define OS2CX_ATTRS_HPP_

#include "mesh.hpp"
#include "poly.hpp"
#include "poly_map.hpp"
#include "poly_map_index.hpp"

namespace os2cx {

class ElementSet {
public:
    std::set<ElementId> elements;
};

ElementSet compute_element_set_from_range(ElementId begin, ElementId end);

ElementSet compute_element_set_from_mask(
    const Poly3Map &poly3_map,
    const Poly3MapIndex &poly3_map_index,
    const Mesh3 &mesh,
    ElementId element_begin,
    ElementId element_end,
    const Poly3 *mask);

class NodeSet {
public:
    std::set<NodeId> nodes;
};

NodeSet compute_node_set_from_range(NodeId begin, NodeId end);

NodeSet compute_node_set_from_element_set(
    const Mesh3 &mesh,
    const ElementSet &element_set);

class ConcentratedLoad {
public:
    class Load {
    public:
        Load() : force(ForceVector::zero()) { }
        ForceVector force;
    };
    std::map<NodeId, Load> loads;
};

ConcentratedLoad compute_load_from_element_set(
    const Mesh3 &mesh,
    const ElementSet &element_set,
    ForceDensityVector force);

} /* namespace os2cx */

#endif

