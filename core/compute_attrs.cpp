#include "compute_attrs.hpp"

namespace os2cx {

static const double direction_angle_epsilon = 1e-6;

PlcNef3 compute_plc_nef_for_solid(const Poly3 &solid) {
    AttrBitset attrs_solid;
    attrs_solid.set(attr_bit_solid());
    PlcNef3 solid_nef = PlcNef3::from_poly(solid);
    solid_nef.binarize(attrs_solid, AttrBitset());
    return solid_nef;
}

void compute_plc_nef_select_volume(
    PlcNef3 *solid_nef,
    const Poly3 &mask,
    AttrBitIndex attr_bit_mask
) {
    /* First, set the mask bit on every solid volume of solid_nef. We don't set
    it on non-solid volumes, faces, edges, or vertices; so it stays zero there,
    and we don't spew bits randomly over things we don't care about. */
    assert(attr_bit_mask != attr_bit_solid());
    solid_nef->map_everywhere([&](AttrBitset bs, PlcNef3::FeatureType ft) {
        if (ft == PlcNef3::FeatureType::Volume && bs[attr_bit_solid()]) {
            bs[attr_bit_mask] = true;
        }
        return bs;
    });

    /* For the mask nef: In the solid volumes of the mask, set all bits true.
    Everywhere else, set all bits except 'bit_index_mask'. So AND-ing this
    with solid_nef will clear the mask bit from solid_nef outside the mask. */
    PlcNef3 mask_nef = PlcNef3::from_poly(mask);
    mask_nef.map_everywhere([&](AttrBitset bs, PlcNef3::FeatureType ft) {
        AttrBitset result;
        result.set();
        if (bs == AttrBitset() || ft != PlcNef3::FeatureType::Volume) {
            result.reset(attr_bit_mask);
        }
        return result;
    });

    *solid_nef = solid_nef->binary_and(mask_nef);
}

void select_external_faces_based_on_direction(
    PlcNef3 *nef,
    Vector direction_vector,
    double direction_angle_tolerance,
    AttrBitIndex attr_bit,
    bool attr_value
) {
    double cos_threshold = cos(direction_angle_tolerance / 180 * M_PI)
        - direction_angle_epsilon;
    assert(attr_bit != attr_bit_solid());
    nef->map_faces([&](
        AttrBitset face_attrs,
        AttrBitset vol1_attrs,
        AttrBitset vol2_attrs,
        Vector normal_towards_vol1
    ) {
        if (direction_vector == Vector::zero()) {
            face_attrs.set(attr_bit, attr_value);
        } else {
            bool vol1_solid = vol1_attrs[attr_bit_solid()];
            bool vol2_solid = vol2_attrs[attr_bit_solid()];
            double dot = direction_vector.dot(normal_towards_vol1);
            if (!vol1_solid && vol2_solid && dot > cos_threshold) {
                face_attrs.set(attr_bit, attr_value);
            } else if (vol1_solid && !vol2_solid && -dot > cos_threshold) {
                face_attrs.set(attr_bit, attr_value);
            }
        }
        return face_attrs;
    });
}

void compute_plc_nef_select_surface_external(
    PlcNef3 *solid_nef,
    const Poly3 &mask,
    Vector direction_vector,
    double direction_angle_tolerance,
    AttrBitIndex attr_bit_mask
) {
    /* First, set the mask bit on every external face of solid_nef that
    satisfies the direction criterion. We don't set it on volumes, edges, or
    vertices, so it stays zero there. */
    select_external_faces_based_on_direction(
        solid_nef,
        direction_vector,
        direction_angle_tolerance,
        attr_bit_mask,
        true
    );

    /* For the mask nef: In all solid parts of the mask, set all bits true.
    In the empty volumes, set all bits except 'bit_index_mask'. So AND-ing this
    with solid_nef will clear the mask bit from solid_nef outside the mask. */
    PlcNef3 mask_nef = PlcNef3::from_poly(mask);
    mask_nef.map_everywhere([&](AttrBitset bs, PlcNef3::FeatureType) {
        AttrBitset result;
        result.set();
        if (bs == AttrBitset()) {
            result.reset(attr_bit_mask);
        }
        return result;
    });

    *solid_nef = solid_nef->binary_and(mask_nef);
}

void compute_plc_nef_select_surface_internal(
    PlcNef3 *solid_nef,
    const Poly3 &mask,
    Vector direction_vector,
    double direction_angle_tolerance,
    AttrBitIndex attr_bit_mask
) {
    /* If direction_angle_tolerance >= 90, then compute_face_set_from_attr_bit()
    won't be able to distinguish between the two sides of the surface */
    assert(direction_angle_tolerance < 90);

    PlcNef3 mask_nef = PlcNef3::from_poly(mask);

    /* First, clear the mask bit on every selected face of the mask. */
    select_external_faces_based_on_direction(
        &mask_nef,
        direction_vector,
        direction_angle_tolerance,
        attr_bit_mask,
        false
    );

    /* On selected faces of the mask, set only the mask bit true. Everywhere
    else, set all bits false. */
    mask_nef.map_everywhere([&](AttrBitset bs, PlcNef3::FeatureType) {
        AttrBitset result;
        if (bs[attr_bit_solid()] && !bs[attr_bit_mask]) {
            result.set(attr_bit_mask);
        }
        return result;
    });

    /* Project mask_nef onto solid_nef */
    *solid_nef = solid_nef->binary_or(mask_nef);

    /* Now clear the mask bit in the non-solid parts of solid_nef */
    solid_nef->map_everywhere([&](AttrBitset bs, PlcNef3::FeatureType) {
        if (!bs[attr_bit_solid()]) {
            bs.reset(attr_bit_mask);
        }
        return bs;
    });
}

void compute_plc_nef_select_node(
    PlcNef3 *solid_nef,
    Point point,
    AttrBitIndex attr_bit_mask
) {
    /* point_nef sets attr_bit_mask at just the one affected point */
    PlcNef3 point_nef = PlcNef3::from_point(point);
    AttrBitset attr_bitset;
    attr_bitset.set(attr_bit_mask);
    point_nef.binarize(attr_bitset, AttrBitset());

    /* Now it's also set in solid_nef */
    *solid_nef = solid_nef->binary_or(point_nef);

    /* If the point landed in an unsolid volume of solid_nef, then unset it */
    solid_nef->map_everywhere([&](AttrBitset bs, PlcNef3::FeatureType ft) {
        if (ft == PlcNef3::FeatureType::Volume && !bs[attr_bit_solid()]) {
            bs.reset(attr_bit_mask);
        }
        return bs;
    });
}

MaxElementSize suggest_max_element_size(const Plc3 &plc) {
    Volume approx_volume = pow(2 * plc.compute_approx_scale(), 3);

    /* Simulating 10,000 elements typically takes a few seconds of CPU time and
    less than a gigabyte of RAM, which makes it a safe default. */
    static const int max_elements = 10000;

    /* Choose max_element_volume such that at most max_elements can fit in
    approx_volume. */
    Volume max_element_volume = approx_volume / max_elements;

    /* Convert to a roughly equivalent element size */
    MaxElementSize max_element_size = pow(max_element_volume, 1 / 3.0);

    return max_element_size;
}

ElementSet compute_element_set_from_range(ElementId begin, ElementId end) {
    ElementSet set;
    for (ElementId id = begin; id != end; ++id) {
        set.elements.insert(id);
    }
    return set;
}

ElementSet compute_element_set_from_attr_bit(
    const Mesh3 &mesh,
    ElementId element_begin,
    ElementId element_end,
    AttrBitIndex attr_bit
) {
    assert(attr_bit != attr_bit_solid());
    ElementSet set;
    for (ElementId eid = element_begin; eid != element_end; ++eid) {
        const Element3 &element = mesh.elements[eid];
        if (element.attrs[attr_bit]) {
            set.elements.insert(eid);
        }
    }
    return set;
}

FaceSet compute_face_set_from_attr_bit(
    const Mesh3 &mesh,
    ElementId element_begin,
    ElementId element_end,
    Vector direction_vector,
    double direction_angle_tolerance,
    AttrBitIndex attr_bit
) {
    double cos_threshold = cos(direction_angle_tolerance / 180 * M_PI)
        - direction_angle_epsilon;
    assert(attr_bit != attr_bit_solid());
    FaceSet set;
    FaceId fid;
    for (fid.element_id = element_begin; fid.element_id != element_end;
            ++fid.element_id) {
        const Element3 &element = mesh.elements[fid.element_id];
        const ElementTypeShape *shape = &element_type_shape(element.type);
        for (fid.face = 0; fid.face < static_cast<int>(shape->faces.size());
                ++fid.face) {
            AttrBitset attrs = element.face_attrs[fid.face];
            if (attrs[attr_bit]) {
                /* This direction_vector check is _almost_ redundant, because
                compute_plc_nef3_select_surface_*() wouldn't have set the attr
                bit if it didn't pass the direction_vector check. The problem is
                that PlcNef3 doesn't keep track of the "orientation" of the
                surface, so for an internal surface selection, we'd end up
                selecting the faces on both sides of the surface. The easiest
                workaround is to re-check direction_vector to disambiguate. */
                Vector face_normal = mesh.oriented_area(element, fid.face);
                face_normal /= face_normal.magnitude();
                double dot = direction_vector.dot(face_normal);
                if (dot > cos_threshold) {
                    set.faces.insert(fid);
                }
            }
        }
    }
    return set;
}

NodeSet compute_node_set_singleton(NodeId node) {
    NodeSet set;
    set.nodes.insert(node);
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

NodeId compute_node_id_from_attr_bit(
    const Mesh3 &mesh,
    NodeId node_begin,
    NodeId node_end,
    AttrBitIndex attr_bit
) {
    NodeId node_id = NodeId::invalid();
    assert(attr_bit != attr_bit_solid());
    for (NodeId nid = node_begin; nid != node_end; ++nid) {
        const Node3 &node = mesh.nodes[nid];
        if (node.attrs[attr_bit]) {
            assert(node_id == NodeId::invalid());
            node_id = nid;
        }
    }
    return node_id;
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
        /* Note: For second-order rectangular faces, some oriented areas may
        point in the opposite direction of the overall face! */
        Vector areas_for_nodes[ElementTypeShape::max_vertices_per_element];
        mesh.oriented_areas_for_nodes(element, face_id.face, areas_for_nodes);
        Vector face_oriented_area = Vector::zero();
        for (int i = 0; i < num_nodes; ++i) {
            face_oriented_area += areas_for_nodes[i];
        }
        double face_area = face_oriented_area.magnitude();
        total_area += face_area;
        for (int i = 0; i < num_nodes; ++i) {
            load.loads[element.nodes[i]].force +=
                areas_for_nodes[i].dot(face_oriented_area) / face_area
                    * force_total_or_per_area;
        }
    }
    if (!force_is_per_area && total_area != 0) {
        for (auto &pair : load.loads) {
            pair.second.force /= total_area;
        }
    }
    return load;
}

void Slice::append_slice(const Slice &other, const MeshIdMapping &id_mapping) {
    pairs.reserve(pairs.size() + other.pairs.size());
    for (const Slice::Pair &other_pair : other.pairs) {
        Slice::Pair pair;
        pair.nodes[0] = id_mapping.convert_node_id(other_pair.nodes[0]);
        pair.nodes[1] = id_mapping.convert_node_id(other_pair.nodes[1]);
        pair.normal = other_pair.normal;
        pairs.push_back(pair);
    }
}

Slice compute_slice(
    Mesh3 *mesh,
    const FaceSet &face_set
) {
    /* For each node that we partition, we'll generate two or more "partitioned
    nodes". After the slice operation is finished, these will just be regular
    nodes; but during the slice operation, we use a different typedef for their
    IDs, to make it easier to keep track of whether we're talking about pre-
    partition or post-partition nodes. The post-partition node ID will also act
    as a unique identifier for the partition itself. */
    typedef NodeId PartitionedNodeId;

    Mesh3Index mesh_index(*mesh);

    /* Identify all nodes that might be participating in the slice */
    std::set<NodeId> nodes;
    for (const FaceId &face : face_set.faces) {
        const Element3 &element = mesh->elements[face.element_id];
        for (int vertex :
                element_type_shape(element.type).faces[face.face].vertices) {
            nodes.insert(element.nodes[vertex]);
        }
    }

    /* For each eligible node, build a data structure tracking all the elements
    it participates in and which faces they share with each other. */
    struct NodeElementData {
        NodeElementData() {
            for (int face = 0; face < ElementTypeShape::max_faces_per_element;
                    ++face) {
                adjacent[face] = ElementId::invalid();
            }
            partitioned_node_id = NodeId::invalid();
        }

        /* Where the node appears in the element */
        int vertex;

        /* For each face on this element: ElementId::invalid() if this node is
        not part of the face or if the face is unmatched; otherwise, the element
        the face matches to. */
        ElementId adjacent[ElementTypeShape::max_faces_per_element];

        /* For each entry in 'adjacent' thats not ElementId::invalid(), true if
        the face in question was sliced */
        bool adjacent_sliced[ElementTypeShape::max_faces_per_element];

        /* Initially PartitionedNodeId::invalid(). After assigning the element
        to a partition, we'll fill this in with the partitioned node ID. */
        PartitionedNodeId partitioned_node_id;
    };
    std::map<NodeId, std::map<ElementId, NodeElementData> > node_element_data;
    for (ElementId element_id = mesh->elements.key_begin();
            element_id != mesh->elements.key_end(); ++element_id) {
        const Element3 &element = mesh->elements[element_id];
        const ElementTypeShape &shape = element_type_shape(element.type);
        for (int face = 0; face < static_cast<int>(shape.faces.size());
                ++face) {
            FaceId face_id_1(element_id, face);
            FaceId face_id_2 = mesh_index.matching_face(face_id_1);
            if (face_id_2 == FaceId::invalid()) {
                continue;
            }
            for (int vertex : shape.faces[face].vertices) {
                NodeId node = element.nodes[vertex];
                if (!nodes.count(node)) {
                    continue;
                }
                NodeElementData *data = &node_element_data[node][element_id];
                data->vertex = vertex;
                data->adjacent[face] = face_id_2.element_id;
                data->adjacent_sliced[face] =
                    face_set.faces.count(face_id_1) ||
                    face_set.faces.count(face_id_2);
            }
        }
    }

    Slice slice;

    /* Visit each node and consider partitioning it */
    for (auto &pair : node_element_data) {
        NodeId node_id = pair.first;
        std::map<ElementId, NodeElementData> &element_data = pair.second;

        /* Generate the partitions, assigning each element to one of the
        partitions. Also create the partitioned nodes. */
        std::vector<PartitionedNodeId> partitioned_node_ids;
        for (auto &data_pair : element_data) {
            ElementId element_id = data_pair.first;
            NodeElementData &data = data_pair.second;
            if (data.partitioned_node_id != PartitionedNodeId::invalid()) {
                /* We've already visited this element */
                continue;
            }

            /* Allocate a node ID for this partition */
            PartitionedNodeId partitioned_node_id;
            if (partitioned_node_ids.empty()) {
                /* Reuse the existing node record */
                partitioned_node_id = node_id;
            } else {
                /* Clone the existing node record with a new ID */
                partitioned_node_id = mesh->nodes.key_end();
                mesh->nodes.push_back(mesh->nodes[node_id]);
            }
            partitioned_node_ids.push_back(partitioned_node_id);

            /* Iteratively find all elements that are reachable from this
            element, and bring them into the partition */
            std::deque<ElementId> queue;
            queue.push_back(element_id);
            while (!queue.empty()) {
                ElementId next_element_id = queue.front();
                queue.pop_front();
                NodeElementData &next_data = element_data[next_element_id];
                if (next_data.partitioned_node_id !=
                        PartitionedNodeId::invalid()) {
                    /* We've already visited this element */
                    continue;
                }
                /* Mark this element with the node ID we chose for this
                partition */
                next_data.partitioned_node_id = partitioned_node_id;
                /* Bring each adjacent non-sliced element into the partition */
                for (int face = 0;
                        face < ElementTypeShape::max_faces_per_element;
                        ++face) {
                    if (next_data.adjacent[face] != ElementId::invalid() &&
                            !next_data.adjacent_sliced[face]) {
                        queue.push_back(next_data.adjacent[face]);
                    }
                }
            }
        }

        /* Update the actual element records to point at the partitioned nodes
        */
        for (const auto &data_pair : element_data) {
            Element3 &element = mesh->elements[data_pair.first];
            assert(element.nodes[data_pair.second.vertex] == node_id);
            element.nodes[data_pair.second.vertex] =
                data_pair.second.partitioned_node_id;
        }

        /* For each sliced face, figure out which pair of partitions it slices
        between, and record the normal of the face, indexed by the pair of
        partitions */
        std::map<
            std::pair<PartitionedNodeId, PartitionedNodeId>,
            std::vector<Vector>
            > partition_pair_areas;
        for (const auto &data_pair : element_data) {
            PartitionedNodeId partition1 = data_pair.second.partitioned_node_id;
            for (int face = 0; face < ElementTypeShape::max_faces_per_element;
                    ++face) {
                ElementId element2 = data_pair.second.adjacent[face];
                if (element2 == ElementId::invalid()) {
                    /* The node we're partitioning isn't part of this face, or
                    this face index is past the max face index of the element */
                    continue;
                }
                PartitionedNodeId partition2 =
                    element_data.at(element2).partitioned_node_id;
                if (partition1 == partition2) {
                    /* These two elements are in the same partition, so the face
                    doesn't slice between two different partitions. */
                    continue;
                }
                assert(data_pair.second.adjacent_sliced[face]);
                if (partition2 < partition1) {
                    /* We'll reach each pair of elements twice; to avoid double-
                    counting, skip based on the partitions' order */
                    continue;
                }
                std::pair<PartitionedNodeId, PartitionedNodeId> key(
                    partition1, partition2);
                Vector oriented_area =
                    mesh->oriented_area(mesh->elements[data_pair.first], face);
                Vector normal = oriented_area / oriented_area.magnitude();
                partition_pair_areas[key].push_back(normal);
            }
        }

        /* Emit slice pair records */
        for (const auto &partition_pair_area_pair : partition_pair_areas) {
            Slice::Pair slice_pair;
            slice_pair.nodes[0] = partition_pair_area_pair.first.first;
            slice_pair.nodes[1] = partition_pair_area_pair.first.second;

            /* Set the slice pair normal to the average of all the face normals.
            This will generally produce correct results, but it's not perfect;
            it would be better to weight the face normals by the angle of the
            vertex on the face. But this should be close enough except in really
            weird cases. */
            Vector normal_sum = Vector::zero();
            for (const Vector &normal : partition_pair_area_pair.second) {
                normal_sum += normal;
            }
            slice_pair.normal =
                normal_sum / partition_pair_area_pair.second.size();

            slice.pairs.push_back(slice_pair);
        }
    }

    return slice;
}

std::vector<LinearEquation> compute_equations_for_slice(const Slice &slice) {
    std::vector<LinearEquation> equations;
    for (const Slice::Pair &pair : slice.pairs) {
        LinearEquation equation;
        equation.terms[LinearEquation::Variable(pair.nodes[0], Dimension::X)] =
            pair.normal.x;
        equation.terms[LinearEquation::Variable(pair.nodes[0], Dimension::Y)] =
            pair.normal.y;
        equation.terms[LinearEquation::Variable(pair.nodes[0], Dimension::Z)] =
            pair.normal.z;
        equation.terms[LinearEquation::Variable(pair.nodes[1], Dimension::X)] =
            -pair.normal.x;
        equation.terms[LinearEquation::Variable(pair.nodes[1], Dimension::Y)] =
            -pair.normal.y;
        equation.terms[LinearEquation::Variable(pair.nodes[1], Dimension::Z)] =
            -pair.normal.z;
        equations.push_back(std::move(equation));
    }
    return equations;
}

} /* namespace os2cx */

