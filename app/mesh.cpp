#include "mesh.hpp"

namespace os2cx {

void Mesh3::append_mesh(
    const Mesh3 &other,
    NodeId *new_node_begin_out,
    NodeId *new_node_end_out,
    ElementId *new_element_begin_out,
    ElementId *new_element_end_out
) {
    int node_id_offset = nodes.key_end().to_int()
        - other.nodes.key_begin().to_int();

    nodes.reserve(nodes.size() + other.nodes.size());
    *new_node_begin_out = nodes.key_end();
    for (const Node3 &node : other.nodes) {
        nodes.push_back(node);
    }
    *new_node_end_out = nodes.key_end();

    elements.reserve(elements.size() + other.elements.size());
    *new_element_begin_out = elements.key_end();
    for (const Element3 &element : other.elements) {
        Element3 copy = element;
        for (int i = 0; i < copy.num_nodes(); ++i) {
            copy.nodes[i] = NodeId::from_int(
                copy.nodes[i].to_int() + node_id_offset);
        }
        elements.push_back(copy);
    }
    *new_element_end_out = elements.key_end();
}

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

} /* namespace os2cx */

