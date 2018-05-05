#include "mesh.hpp"

namespace os2cx {

Volume Mesh3::volume(const Element3 &element) const {
    const ElementShapeInfo &shape =
        *ElementTypeInfo::get(element.type).shape;
    double vol = evaluate_polynomial(element, shape.volume_function);
    return Volume(vol);
}

Volume Mesh3::volume_for_node(const Element3 &element, int which) const {
    const ElementShapeInfo &shape =
        *ElementTypeInfo::get(element.type).shape;
    double vol = evaluate_polynomial(
        element,
        shape.vertices[which].volume_function);
    return Volume(vol);
}

double Mesh3::evaluate_polynomial(
    const Element3 &element,
    const Polynomial &poly
) const {
    return poly.evaluate(
        [&](Polynomial::Variable var) {
            int vertex = shape_function_variables::coord_to_vertex(var);
            Point point = nodes[element.nodes[vertex]].point;
            switch (shape_function_variables::coord_to_dimension(var)) {
            case 0: return point.vector.x.val;
            case 1: return point.vector.y.val;
            case 2: return point.vector.z.val;
            default: assert(false);
            }
        }
    );
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

