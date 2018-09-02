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

class ElementNodeLocs {
public:
    ElementNodeLocs(const Mesh3 &mesh, const Element3 &element) {
        for (int node = 0; node < element.num_nodes(); ++node) {
            Point point = mesh.nodes[element.nodes[node]].point;
            vars[shape_function_variables::coord(node, 0).index] = point.x;
            vars[shape_function_variables::coord(node, 1).index] = point.y;
            vars[shape_function_variables::coord(node, 2).index] = point.z;
        }
    }

    double evaluate(const Polynomial &poly, const double *uvw) {
        vars[shape_function_variables::u().index] = uvw[0];
        vars[shape_function_variables::v().index] = uvw[1];
        vars[shape_function_variables::w().index] = uvw[2];
        return poly.evaluate(
            [&](Polynomial::Variable var) {
                return vars[var.index];
            }
        );
    }

    double vars[shape_function_variables::max_var_plus_one];
};

double jacobian_determinant(
    ElementNodeLocs *enl,
    const ElementShapeInfo &shape,
    const double *uvw
) {
    double jac[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            jac[i][j] = enl->evaluate(shape.jacobian[i][j], uvw);
        }
    }
    return jac[0][0] * jac[1][1] * jac[2][2]
         + jac[1][0] * jac[2][1] * jac[0][2]
         + jac[2][0] * jac[0][1] * jac[1][2]
         - jac[0][0] * jac[2][1] * jac[1][2]
         - jac[1][0] * jac[0][1] * jac[2][2]
         - jac[2][0] * jac[1][1] * jac[0][2];
}

template<class Callable>
void integrate(
    ElementNodeLocs *enl,
    const ElementShapeInfo &shape,
    const Callable &callable
) {
    for (const ElementShapeInfo::IntegrationPoint &ip :
            shape.integration_points) {
        double jac_det = jacobian_determinant(enl, shape, ip.shape_uvw);
        callable(ip.shape_uvw, jac_det * ip.weight);
    }
}

Volume Mesh3::volume(const Element3 &element) const {
    ElementNodeLocs enl(*this, element);
    const ElementShapeInfo &shape =
        *ElementTypeInfo::get(element.type).shape;
    double total_volume = 0;
    integrate(&enl, shape, [&](const double *, double v) {
        total_volume += v;
    });
    return Volume(total_volume);
}

void Mesh3::volumes_for_nodes(
    const Element3 &element,
    Volume *volumes_out
) const {
    ElementNodeLocs enl(*this, element);
    const ElementShapeInfo &shape =
        *ElementTypeInfo::get(element.type).shape;
    for (int i = 0; i < static_cast<int>(shape.vertices.size()); ++i) {
        volumes_out[i] = 0;
    }
    integrate(&enl, shape, [&](const double *uvw, double v) {
        for (int i = 0; i < static_cast<int>(shape.vertices.size()); ++i) {
            volumes_out[i] += v * enl.evaluate(
                shape.vertices[i].shape_function,
                uvw
            );
        }
    });
}

} /* namespace os2cx */

