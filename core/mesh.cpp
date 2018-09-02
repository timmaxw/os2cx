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

double Mesh3::jacobian_determinant(
    const Element3 &element,
    const double *uvw
) const {
    const ElementShapeInfo &shape = *ElementTypeInfo::get(element.type).shape;
    double sf_d_uvw[ElementShapeInfo::max_vertices_per_element * 3];
    shape.shape_function_derivatives(uvw, sf_d_uvw);
    double jacobian[3][3] = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };
    for (int i = 0; i < static_cast<int>(shape.vertices.size()); ++i) {
        Point point = nodes[element.nodes[i]].point;
        for (int j = 0; j < 3; ++j) {
            jacobian[0][j] += point.x * sf_d_uvw[i * 3 + j];
            jacobian[1][j] += point.y * sf_d_uvw[i * 3 + j];
            jacobian[2][j] += point.z * sf_d_uvw[i * 3 + j];
        }
    }
    return jacobian[0][0] * jacobian[1][1] * jacobian[2][2]
         + jacobian[1][0] * jacobian[2][1] * jacobian[0][2]
         + jacobian[2][0] * jacobian[0][1] * jacobian[1][2]
         - jacobian[0][0] * jacobian[2][1] * jacobian[1][2]
         - jacobian[1][0] * jacobian[0][1] * jacobian[2][2]
         - jacobian[2][0] * jacobian[1][1] * jacobian[0][2];
}

Volume Mesh3::volume(const Element3 &element) const {
    const ElementShapeInfo &shape = *ElementTypeInfo::get(element.type).shape;
    double total_volume = 0;
    for (const ElementShapeInfo::IntegrationPoint &ip :
            shape.integration_points) {
        total_volume += ip.weight * jacobian_determinant(element, ip.shape_uvw);
    }
    return Volume(total_volume);
}

void Mesh3::volumes_for_nodes(
    const Element3 &element,
    Volume *volumes_out
) const {
    const ElementShapeInfo &shape =
        *ElementTypeInfo::get(element.type).shape;
    for (int i = 0; i < static_cast<int>(shape.vertices.size()); ++i) {
        volumes_out[i] = 0;
    }
    for (const ElementShapeInfo::IntegrationPoint &ip :
            shape.integration_points) {
        double weight = ip.weight * jacobian_determinant(element, ip.shape_uvw);
        double sf[ElementShapeInfo::max_vertices_per_element];
        shape.shape_functions(ip.shape_uvw, sf);
        for (int i = 0; i < static_cast<int>(shape.vertices.size()); ++i) {
            volumes_out[i] += weight * sf[i];
        }
    }
}

} /* namespace os2cx */

