#include "attrs.hpp"

namespace os2cx {

Plc3::BitIndex bit_index_solid() {
    return 0;
}

PlcNef3 compute_plc_nef_for_solid(const Poly3 &solid) {
    Plc3::Bitset bitset_solid;
    bitset_solid.set(bit_index_solid());
    PlcNef3 solid_nef = PlcNef3::from_poly(solid);
    solid_nef.everywhere_binarize(bitset_solid);
    return solid_nef;
}

void compute_plc_nef_select_volume(
    PlcNef3 *solid_nef, const Poly3 &mask, Plc3::BitIndex bit_index_mask
) {
    /* First, set the mask bit on every solid volume of solid_nef. We don't set
    it on non-solid volumes, faces, edges, or vertices; so it stays zero there,
    and we don't spew bits randomly over things we don't care about. */
    assert(bit_index_mask != bit_index_solid());
    solid_nef->everywhere_map([&](Plc3::Bitset bs, PlcNef3::FeatureType ft) {
        if (ft == PlcNef3::FeatureType::Volume && bs[bit_index_solid()]) {
            bs[bit_index_mask] = true;
        }
        return bs;
    });

    /* For the mask nef: In the solid volumes of the mask, set all bits true.
    Everywhere else, set all bits except 'bit_index_mask'. So AND-ing this
    with solid_nef will clear the mask bit from solid_nef outside the mask. */
    PlcNef3 mask_nef = PlcNef3::from_poly(mask);
    mask_nef.everywhere_map([&](Plc3::Bitset bs, PlcNef3::FeatureType ft) {
        Plc3::Bitset result;
        result.set();
        if (bs == Plc3::Bitset() || ft != PlcNef3::FeatureType::Volume) {
            result.reset(bit_index_mask);
        }
        return result;
    });

    *solid_nef = solid_nef->binary_and(mask_nef);
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
        if (bitset[bit_index]) {
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

