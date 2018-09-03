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
    const ElementShapeInfo &shape = *ElementTypeInfo::get(element.type).shape;
    double total_volume = 0;
    for (const auto &ip : shape.volume_integration_points) {
        total_volume += integrate_volume(element, ip);
    }
    return Volume(total_volume);
}

void Mesh3::volumes_for_nodes(
    const Element3 &element,
    Volume *volumes_out
) const {
    const ElementShapeInfo &shape = *ElementTypeInfo::get(element.type).shape;
    for (int i = 0; i < static_cast<int>(shape.vertices.size()); ++i) {
        volumes_out[i] = 0;
    }
    for (const auto &ip : shape.volume_integration_points) {
        double d_volume = integrate_volume(element, ip);
        double sf[ElementShapeInfo::max_vertices_per_element];
        shape.shape_functions(ip.uvw, sf);
        for (int i = 0; i < static_cast<int>(shape.vertices.size()); ++i) {
            volumes_out[i] += sf[i] * d_volume;
        }
    }
}

Vector Mesh3::oriented_area(const Element3 &element, int face_index) const {
    const ElementShapeInfo &shape = *ElementTypeInfo::get(element.type).shape;
    const ElementShapeInfo::Face &face_shape = shape.faces[face_index];
    Vector total_oriented_area = Vector::zero();
    for (const auto &ip : face_shape.integration_points) {
        total_oriented_area += integrate_area(element, face_index, ip);
    }
    return total_oriented_area;
}

void Mesh3::oriented_areas_for_nodes(
    const Element3 &element,
    int face_index,
    Vector *areas_out
) const {
    const ElementShapeInfo &shape = *ElementTypeInfo::get(element.type).shape;
    const ElementShapeInfo::Face &face_shape = shape.faces[face_index];
    for (int i = 0; i < static_cast<int>(shape.vertices.size()); ++i) {
        areas_out[i] = Vector::zero();
    }
    for (const auto &ip : face_shape.integration_points) {
        Vector d_area = integrate_area(element, face_index, ip);
        double sf[ElementShapeInfo::max_vertices_per_element];
        shape.shape_functions(ip.uvw, sf);
        for (int node_ix : face_shape.vertices) {
            areas_out[node_ix] += sf[node_ix] * d_area;
        }
    }
}

Matrix Mesh3::jacobian(
    const Element3 &element,
    ElementShapeInfo::ShapePoint uvw
) const {
    const ElementShapeInfo &shape = *ElementTypeInfo::get(element.type).shape;
    ElementShapeInfo::ShapeVector sf_d_uvw[
        ElementShapeInfo::max_vertices_per_element];
    shape.shape_function_derivatives(uvw, sf_d_uvw);
    Matrix jacobian = Matrix::zero();
    for (int i = 0; i < static_cast<int>(shape.vertices.size()); ++i) {
        Vector point = nodes[element.nodes[i]].point - Point::origin();
        jacobian.cols[0] += point * sf_d_uvw[i].x;
        jacobian.cols[1] += point * sf_d_uvw[i].y;
        jacobian.cols[2] += point * sf_d_uvw[i].z;
    }
    return jacobian;
}

double Mesh3::integrate_volume(
    const Element3 &element,
    const ElementShapeInfo::IntegrationPoint &ip
) const {
    return ip.weight * jacobian(element, ip.uvw).determinant();
}

Vector Mesh3::integrate_area(
    const Element3 &element,
    int face_index,
    const ElementShapeInfo::IntegrationPoint &ip
) const {
    Matrix j = jacobian(element, ip.uvw);
    ElementShapeInfo::ShapeVector shape_normal =
        ElementTypeInfo::get(element.type).shape->faces[face_index].normal;
    return ip.weight * j.cofactor_matrix().apply(shape_normal);
}

} /* namespace os2cx */

