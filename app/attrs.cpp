#include "attrs.hpp"

namespace os2cx {

ElementSet compute_element_set_from_range(ElementId begin, ElementId end) {
    ElementSet set;
    for (ElementId id = begin; id != end; ++id) {
        set.elements.insert(id);
    }
    return set;
}

ElementSet compute_element_set_from_mask(
    const Poly3Map &poly3_map,
    const Poly3MapIndex &poly3_map_index,
    const Mesh3 &mesh,
    ElementId element_begin,
    ElementId element_end,
    const Poly3 *mask
) {
    ElementSet set;
    for (ElementId eid = element_begin; eid != element_end; ++eid) {
        const Element3 &element = mesh.elements[eid];
        LengthVector c = LengthVector::zero();
        int num_nodes = element.num_nodes();
        for (int i = 0; i < num_nodes; ++i) {
            c += mesh.nodes[element.nodes[i]].point.vector;
        }
        c /= num_nodes;
        const Poly3Map::Volume &volume = poly3_map.volumes[
            poly3_map_index.volume_containing_point(Point(c))];
        assert(volume.is_solid);
        if (volume.masks.find(mask)->second) {
            set.elements.insert(eid);
        }
    }
    return set;
}

NodeSet compute_node_set_from_range(NodeId begin, NodeId end) {
    NodeSet set;
    for (NodeId id = begin; id != end; ++id) {
        set.nodes.insert(id);
    }
    return set;
}

NodeSet compute_node_set_from_element_set(
    const Mesh3 &mesh,
    const ElementSet &element_set
) {
    NodeSet set;
    for (ElementId element_id : element_set.elements) {
        const Element3 &element = mesh.elements[element_id];
        int num_nodes = element.num_nodes();
        for (int i = 0; i < num_nodes; ++i) {
            set.nodes.insert(element.nodes[i]);
        }
    }
    return set;
}

ConcentratedLoad compute_load_from_element_set(
    const Mesh3 &mesh,
    const ElementSet &element_set,
    ForceDensityVector force
) {
    ConcentratedLoad load;
    for (ElementId element_id : element_set.elements) {
        const Element3 &element = mesh.elements[element_id];
        int num_nodes = element.num_nodes();
        for (int i = 0; i < num_nodes; ++i) {
            Volume vol = mesh.volume_for_node(element, i);
            load.loads[element.nodes[i]].force += vol * force;
        }
    }
    return load;
}

} /* namespace os2cx */

