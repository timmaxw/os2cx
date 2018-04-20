#include "mesh_index.hpp"

#include <boost/container/flat_map.hpp>

namespace os2cx {

class FaceNodes {
public:
    static FaceNodes make(const Element3 &element, int face) {
        FaceNodes out;

        const ElementShapeInfo &esi =
            *ElementTypeInfo::get(element.type).shape;
        assert(face < static_cast<int>(esi.faces.size()));
        out.num_nodes = esi.faces[face].size();

        /* Find the index of the node on the face with the largest node ID */
        int largest_node_id = -1;
        int largest_index = -1;
        for (int i = 0; i < out.num_nodes; ++i) {
            int node_id = element.nodes[esi.faces[face][i]].to_int();
            if (node_id > largest_node_id) {
                largest_node_id = node_id;
                largest_index = i;
            }
        }
        assert(largest_index != -1);

        /* Copy the node IDs into 'nodes', starting from the one with the
        largest node ID. This ensures that if there's another face that matches
        this one, it will start from the same point (except reversed). */
        for (int i = 0; i < out.num_nodes; ++i) {
            int j = (largest_index + i) % out.num_nodes;
            out.nodes[i] = element.nodes[esi.faces[face][j]];
        }

        return out;
    }

    void reverse() {
        /* Switch the direction of the cycle, maintaining the rule that the
        largest-indexed node remains in the first slot. */
        for (int i = 1; i <= num_nodes / 2; ++i) {
            std::swap(nodes[i], nodes[num_nodes - i]);
        }
    }

    /* This is just an arbitrary ordering so we can use FaceNodes as a sorting
    key */
    bool operator<(const FaceNodes &o) const {
        if (num_nodes < o.num_nodes) return true;
        if (o.num_nodes < num_nodes) return false;
        for (int i = 0; i < num_nodes; ++i) {
            if (nodes[i].to_int() < o.nodes[i].to_int()) return true;
            if (o.nodes[i].to_int() < nodes[i].to_int()) return false;
        }
        return false;
    }

    int num_nodes;
    NodeId nodes[ElementShapeInfo::max_nodes_per_face];
};

Mesh3Index::Mesh3Index(const Mesh3 *mesh_) : mesh(mesh_) {
    /* We want to build a map from FaceNodes to FaceId so that we can match
    faces. We use 'boost::container::flat_map' instead of 'std::map' in order to
    reduce memory allocations and improve memory locality. The FaceNodes
    ordering function should put matching faces close together in the map, which
    will further improve locality. */
    typedef boost::container::flat_map<FaceNodes, FaceId> MapType;
    MapType::sequence_type sequence;
    for (ElementId element_id = mesh->elements.key_begin();
            element_id != mesh->elements.key_end();
            ++element_id) {
        const Element3 &element = mesh->elements[element_id];
        const ElementShapeInfo &esi =
            *ElementTypeInfo::get(element.type).shape;
        for (int face = 0; face < static_cast<int>(esi.faces.size()); ++face) {
            sequence.push_back(std::make_pair(
                FaceNodes::make(element, face),
                FaceId { element_id, face }
            ));
        }
    }

    int num_faces = sequence.size();
    MapType map;
    map.adopt_sequence(std::move(sequence));

    /* This would fail if some faces were non-unique, i.e. if two elements
    shared a certain face *from the same side*. */
    assert(num_faces == static_cast<int>(map.size()));

    matching_faces = ContiguousMap<FaceId, FaceId>(
        FaceId { mesh->elements.key_begin(), 0 },
        FaceId { mesh->elements.key_end(), 0 },
        FaceId::invalid()
    );

    for (auto it = map.begin(); it != map.end(); ++it) {
        FaceId *result = &matching_faces[it->second];
        FaceNodes key = it->first;
        key.reverse();
        auto jt = map.find(key);
        if (jt == map.end()) {
            *result = FaceId::invalid();
            unmatched_faces.push_back(it->second);
        } else {
            *result = jt->second;
        }
    }
}

} /* namespace os2cx */

