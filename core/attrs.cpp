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

void compute_plc_nef_select_surface(
    PlcNef3 *solid_nef, const Poly3 &mask, Plc3::BitIndex bit_index_mask
) {
    /* First, set the mask bit on every solid face/edge/vertex of solid_nef. We
    don't set it on volumes or on non-solid faces/edges/vertices, so it stays
    zero there. In particular, we won't create internal structures inside of
    existing volumes. */
    assert(bit_index_mask != bit_index_solid());
    solid_nef->everywhere_map([&](Plc3::Bitset bs, PlcNef3::FeatureType ft) {
        if (ft != PlcNef3::FeatureType::Volume && bs[bit_index_solid()]) {
            bs[bit_index_mask] = true;
        }
        return bs;
    });

    /* For the mask nef: In all solid parts of the mask, set all bits true.
    In the empty volumes, set all bits except 'bit_index_mask'. So AND-ing this
    with solid_nef will clear the mask bit from solid_nef outside the mask. */
    PlcNef3 mask_nef = PlcNef3::from_poly(mask);
    mask_nef.everywhere_map([&](Plc3::Bitset bs, PlcNef3::FeatureType) {
        Plc3::Bitset result;
        result.set();
        if (bs == Plc3::Bitset()) {
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

FaceSet compute_face_set_from_plc_bit(
    const Plc3Index &plc_index,
    const Mesh3 &mesh,
    const Mesh3Index &mesh_index,
    ElementId element_begin,
    ElementId element_end,
    Plc3::BitIndex bit_index
) {
    assert(bit_index != bit_index_solid());
    FaceSet set;
    FaceId fid;
    for (fid.element_id = element_begin; fid.element_id != element_end;
            ++fid.element_id) {
        const Element3 &element = mesh.elements[fid.element_id];
        const ElementShapeInfo *shape =
            ElementTypeInfo::get(element.type).shape;
        for (fid.face = 0; fid.face < static_cast<int>(shape->faces.size());
                ++fid.face) {
            FaceId other_fid = mesh_index.matching_face(fid);
            if (other_fid != FaceId::invalid()) {
                /* internal face, skip */
                continue;
            }

            LengthVector c = LengthVector::zero();
            for (int vertex_index : shape->faces[fid.face].vertices) {
                c += mesh.nodes[element.nodes[vertex_index]].point.vector;
            }
            c /= shape->faces[fid.face].vertices.size();

            Plc3::SurfaceId surface_id =
                plc_index.surface_closest_to_point(Point(c));
            Plc3::Bitset bitset = plc_index.plc->surfaces[surface_id].bitset;
            if (bitset[bit_index]) {
                set.faces.insert(fid);
            }
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

NodeSet compute_node_set_from_face_set(
    const Mesh3 &mesh,
    const FaceSet &face_set
) {
    NodeSet set;
    for (FaceId face_id : face_set.faces) {
        const Element3 &element = mesh.elements[face_id.element_id];
        const ElementShapeInfo *shape =
            ElementTypeInfo::get(element.type).shape;

        for (int vertex_index : shape->faces[face_id.face].vertices) {
            set.nodes.insert(element.nodes[vertex_index]);
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

