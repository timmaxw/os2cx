#include "attrs.hpp"

namespace os2cx {

Plc3::BitIndex bit_index_solid() {
    return 0;
}

PlcNef3 compute_plc_nef_for_solid(const Poly3 &solid) {
    /* In solid regions, we set bit_index_solid() and clear all other bits. In
    space regions, we do the exact opposite. This ensures that when
    compute_plc_nef_select_volume() calls solid_nef.binary_or(mask_nef) then the
    space regions won't be modified. */
    Plc3::Bitset bitset_solid, bitset_space;
    bitset_solid.set(bit_index_solid());
    bitset_space = ~bitset_solid;
    PlcNef3 solid_nef = PlcNef3::from_poly(solid);
    solid_nef.everywhere_map([&](Plc3::Bitset bitset, PlcNef3::FeatureType) {
        return (bitset != Plc3::Bitset()) ? bitset_solid : bitset_space;
    });
    return solid_nef;
}

void compute_plc_nef_select_volume(
    PlcNef3 *solid_nef, const Poly3 &mask, Plc3::BitIndex bit_index_mask
) {
    assert(bit_index_mask != bit_index_solid());
    Plc3::Bitset bitset_mask;
    bitset_mask.set(bit_index_mask);
    PlcNef3 mask_nef = PlcNef3::from_poly(mask);
    mask_nef.everywhere_binarize(bitset_mask);
    *solid_nef = solid_nef->binary_or(mask_nef);
}

ElementSet compute_element_set_from_range(ElementId begin, ElementId end) {
    ElementSet set;
    for (ElementId id = begin; id != end; ++id) {
        set.elements.insert(id);
    }
    return set;
}

ElementSet compute_element_set_from_plc_bit(
    const Plc3Index &plc_index,
    const Mesh3 &mesh,
    ElementId element_begin,
    ElementId element_end,
    Plc3::BitIndex bit_index
) {
    assert(bit_index != bit_index_solid());
    ElementSet set;
    for (ElementId eid = element_begin; eid != element_end; ++eid) {
        const Element3 &element = mesh.elements[eid];
        LengthVector c = LengthVector::zero();
        int num_nodes = element.num_nodes();
        for (int i = 0; i < num_nodes; ++i) {
            c += mesh.nodes[element.nodes[i]].point.vector;
        }
        c /= num_nodes;
        Plc3::VolumeId volume_id = plc_index.volume_containing_point(Point(c));
        Plc3::Bitset bitset = plc_index.plc->volumes[volume_id].bitset;
        if (bitset[bit_index_solid()] && bitset[bit_index]) {
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

