#include "mesh_type_info.hpp"

#include <assert.h>

#include <stdexcept>

namespace os2cx {

using namespace shape_function_variables;

void ElementShapeInfo::precompute_functions() {
    /* xyz[0] is a polynomial in terms of (u, v, w), and the x-coordinates of
    the vertices, which computes the x-coordinate of the point at element
    coordinates (u, v, w). Similarly with xyz[1] for the y-coordinate and xyz[2]
    for the z-coordinate. */
    Polynomial xyz[3];
    for (int vertex = 0; vertex < static_cast<int>(vertices.size()); ++vertex) {
        for (int dimension = 0; dimension < 3; ++dimension) {
            xyz[dimension] += p(coord(vertex, dimension)) *
                vertices[vertex].shape_function;
        }
    }

    /* jac is the Jacobian matrix for the transformation from (u, v, w)
    coordinates to (x, y, z) coordinates. Each element is a polynomial in (u, v,
    w) and the coordinates of the vertices. */
    Polynomial jac[3][3];
    Polynomial::Variable uvw[3] = {u(), v(), w()};
    jacobian(&xyz, uvw, &jac);

    /* jac_det is the determinant of the Jacobian, a polynomial in (u, v, w) and
    the coordinates of the vertices. It is equal to the ratio between the
    infinitesimal volume element dx*dy*dz and the infinitesimal volume element
    du*dv*dw. */
    Polynomial jac_det = determinant(&jac);

    /* Integrating the determinant of the Jacobian over the (u, v, w) space
    spanned by the element, gives the volume of the element in (x, y, z)
    coordinates. This is a polynomial in the coordinates of the vertices and not
    in (u, v, w). */
    volume_function = integrate_uvw(jac_det);

    /* Integrating (jac_det * shape_function) over the (u, v, w) space gives the
    weighted volume of the part of the element influenced by that vertex. */
    for (int vertex = 0; vertex < static_cast<int>(vertices.size()); ++vertex) {
        vertices[vertex].volume_function = integrate_uvw(
            vertices[vertex].shape_function * jac_det);
    }
}

class ElementShapeInfoTetrahedron4 : public ElementShapeInfo {
public:
    ElementShapeInfoTetrahedron4() {
        vertices.resize(4);
        Vertex::Type c = Vertex::Type::Corner;
        vertices[0] = Vertex(c, 0, 0, 0, p(1) - p(u()) - p(v()) - p(w()));
        vertices[1] = Vertex(c, 1, 0, 0, p(u()));
        vertices[2] = Vertex(c, 0, 1, 0, p(v()));
        vertices[3] = Vertex(c, 0, 0, 1, p(w()));

        faces.resize(4);
        faces[0].vertices = { 0, 2, 1 };
        faces[1].vertices = { 0, 1, 3 };
        faces[2].vertices = { 1, 2, 3 };
        faces[3].vertices = { 0, 3, 2 };

        precompute_functions();
    }

    Polynomial integrate_uvw(const Polynomial &poly) const {
        return poly
            .integrate(u(), p(0), p(1) - p(v()) - p(w()))
            .integrate(v(), p(0), p(1) - p(w()))
            .integrate(w(), p(0), p(1));
    }
};

const ElementShapeInfo &element_shape_tetrahedron4() {
    static const ElementShapeInfoTetrahedron4 info;
    return info;
}

bool valid_element_type(ElementType type) {
    switch (type) {
    case ElementType::C3D4:
        return true;
    default:
        return false;
    }
}

ElementType element_type_from_string(const std::string &str) {
    if (str == "C3D4") return ElementType::C3D4;
    throw std::domain_error("no such element type: " + str);
}

const ElementTypeInfo &ElementTypeInfo::get(ElementType type) {
    switch (type) {
    case ElementType::C3D4: return element_type_c3d4();
    default: assert(false);
    }
}

const ElementTypeInfo &element_type_c3d4() {
    static ElementTypeInfo info {
        ElementType::C3D4,
        "C3D4",
        &element_shape_tetrahedron4()
    };
    return info;
}

} /* namespace os2cx */

