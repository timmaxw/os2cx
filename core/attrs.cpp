#include "attrs.hpp"

namespace os2cx {

Plc3::BitIndex bit_index_solid() {
    return 0;
}

PlcNef3 compute_plc_nef_for_solid(const Poly3 &solid) {
    Plc3::Bitset bitset_solid;
    bitset_solid.set(bit_index_solid());
    PlcNef3 solid_nef = PlcNef3::from_poly(solid);
    solid_nef.binarize(bitset_solid, Plc3::Bitset());
    return solid_nef;
}

void compute_plc_nef_select_volume(
    PlcNef3 *solid_nef,
    const Poly3 &mask,
    Plc3::BitIndex bit_index_mask
) {
    /* First, set the mask bit on every solid volume of solid_nef. We don't set
    it on non-solid volumes, faces, edges, or vertices; so it stays zero there,
    and we don't spew bits randomly over things we don't care about. */
    assert(bit_index_mask != bit_index_solid());
    solid_nef->map_everywhere([&](Plc3::Bitset bs, PlcNef3::FeatureType ft) {
        if (ft == PlcNef3::FeatureType::Volume && bs[bit_index_solid()]) {
            bs[bit_index_mask] = true;
        }
        return bs;
    });

    /* For the mask nef: In the solid volumes of the mask, set all bits true.
    Everywhere else, set all bits except 'bit_index_mask'. So AND-ing this
    with solid_nef will clear the mask bit from solid_nef outside the mask. */
    PlcNef3 mask_nef = PlcNef3::from_poly(mask);
    mask_nef.map_everywhere([&](Plc3::Bitset bs, PlcNef3::FeatureType ft) {
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
    PlcNef3 *solid_nef,
    const Poly3 &mask,
    Vector direction_vector,
    double direction_angle_tolerance,
    Plc3::BitIndex bit_index_mask
) {
    double cos_threshold = cos(direction_angle_tolerance / 180 * M_PI);

    /* First, set the mask bit on every external face of solid_nef that
    satisfies the direction criterion. We don't set it on volumes, edges, or
    vertices, so it stays zero there. */
    assert(bit_index_mask != bit_index_solid());
    solid_nef->map_faces([&](
        Plc3::Bitset face_bitset,
        Plc3::Bitset vol1_bitset,
        Plc3::Bitset vol2_bitset,
        Vector normal_towards_vol1
    ) {
        if (direction_vector == Vector::zero()) {
            face_bitset.set(bit_index_mask);
        } else {
            bool vol1_solid = vol1_bitset[bit_index_solid()];
            bool vol2_solid = vol2_bitset[bit_index_solid()];
            double dot = direction_vector.dot(normal_towards_vol1);
            if (!vol1_solid && vol2_solid && dot > cos_threshold) {
                face_bitset.set(bit_index_mask);
            } else if (vol1_solid && !vol2_solid && -dot > cos_threshold) {
                face_bitset.set(bit_index_mask);
            }
        }
        return face_bitset;
    });

    /* For the mask nef: In all solid parts of the mask, set all bits true.
    In the empty volumes, set all bits except 'bit_index_mask'. So AND-ing this
    with solid_nef will clear the mask bit from solid_nef outside the mask. */
    PlcNef3 mask_nef = PlcNef3::from_poly(mask);
    mask_nef.map_everywhere([&](Plc3::Bitset bs, PlcNef3::FeatureType) {
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

        LengthVector sum = LengthVector::zero();
        int num_nodes = element.num_nodes();
        for (int i = 0; i < num_nodes; ++i) {
            sum += mesh.nodes[element.nodes[i]].point - Point::origin();
        }
        Point center = Point::origin() + sum / num_nodes;

        Plc3::VolumeId volume_id = plc_index.volume_containing_point(center);
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
        const ElementTypeShape *shape = &element_type_shape(element.type);
        for (fid.face = 0; fid.face < static_cast<int>(shape->faces.size());
                ++fid.face) {
            FaceId other_fid = mesh_index.matching_face(fid);
            if (other_fid != FaceId::invalid()) {
                /* internal face, skip */
                continue;
            }

            LengthVector sum = LengthVector::zero();
            for (int vertex_index : shape->faces[fid.face].vertices) {
                sum += mesh.nodes[element.nodes[vertex_index]].point
                    - Point::origin();
            }
            Point center = Point::origin()
                + sum / shape->faces[fid.face].vertices.size();

            Plc3::SurfaceId surface_id =
                plc_index.surface_closest_to_point(center);
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
        const ElementTypeShape *shape = &element_type_shape(element.type);

        for (int vertex_index : shape->faces[face_id.face].vertices) {
            set.nodes.insert(element.nodes[vertex_index]);
        }
    }
    return set;
}

ConcentratedLoad compute_load_from_element_set(const Mesh3 &mesh,
    const ElementSet &element_set,
    Vector force_total_or_per_volume,
    bool force_is_per_volume
) {
    ConcentratedLoad load;
    double total_volume = 0;
    for (ElementId element_id : element_set.elements) {
        const Element3 &element = mesh.elements[element_id];
        int num_nodes = element.num_nodes();
        Volume volumes_for_nodes[ElementTypeShape::max_vertices_per_element];
        mesh.volumes_for_nodes(element, volumes_for_nodes);
        for (int i = 0; i < num_nodes; ++i) {
            total_volume += volumes_for_nodes[i];
            load.loads[element.nodes[i]].force +=
                volumes_for_nodes[i] * force_total_or_per_volume;
        }
    }
    if (!force_is_per_volume && total_volume != 0) {
        for (auto &pair : load.loads) {
            pair.second.force /= total_volume;
        }
    }
    return load;
}

ConcentratedLoad compute_load_from_face_set(
    const Mesh3 &mesh,
    const FaceSet &face_set,
    Vector force_total_or_per_area,
    bool force_is_per_area
) {
    ConcentratedLoad load;
    double total_area = 0;
    for (FaceId face_id : face_set.faces) {
        const Element3 &element = mesh.elements[face_id.element_id];
        int num_nodes = element.num_nodes();
        Vector areas_for_nodes[ElementTypeShape::max_vertices_per_element];
        mesh.oriented_areas_for_nodes(element, face_id.face, areas_for_nodes);
        for (int i = 0; i < num_nodes; ++i) {
            total_area += areas_for_nodes[i].magnitude();
            load.loads[element.nodes[i]].force +=
                areas_for_nodes[i].magnitude() * force_total_or_per_area;
        }
    }
    if (!force_is_per_area && total_area != 0) {
        for (auto &pair : load.loads) {
            pair.second.force /= total_area;
        }
    }
    return load;
}

} /* namespace os2cx */

