#include "mesh.hpp"

namespace os2cx {

Volume Mesh3::volume(const Element3 &element) const {
    const ElementShapeInfo &shape =
        *ElementTypeInfo::get(element.type).shape;
    Point points[20];
    for (size_t i = 0; i < shape.nodes.size(); ++i) {
        points[i] = nodes[element.nodes[i]].point;
    }
    return shape.volume(points);
}

Volume Mesh3::volume_for_node(const Element3 &element, int which) const {
    const ElementShapeInfo &shape =
        *ElementTypeInfo::get(element.type).shape;
    Point points[20];
    for (size_t i = 0; i < shape.nodes.size(); ++i) {
        points[i] = nodes[element.nodes[i]].point;
    }
    return shape.volume_for_node(which, points);
}

Mesh3 MeshIdAllocator::allocate(Mesh3 &&mesh) {
    for (Element3 &element : mesh.elements) {
        size_t num_nodes = element.num_nodes();
        for (size_t i = 0; i < num_nodes; ++i) {
            element.nodes[i] = NodeId::from_int(
                element.nodes[i].to_int() +
                (next_node_id.to_int() - mesh.nodes.key_begin().to_int()));
        }
    }

    mesh.nodes.shift_keys(next_node_id);
    next_node_id = mesh.nodes.key_end();

    mesh.elements.shift_keys(next_element_id);
    next_element_id = mesh.elements.key_end();

    return std::move(mesh);
}

} /* namespace os2cx */

