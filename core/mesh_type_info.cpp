#include "mesh_type_info.hpp"

#include <assert.h>

#include <stdexcept>

namespace os2cx {

using namespace shape_function_variables;

/* Abbreviations for common polynomials terms built from shape functions
variables, so that shape functions aren't horribly verbose */
inline Polynomial p(double a) { return Polynomial(a); }
const Polynomial pt =
    p(1) - Polynomial(u()) - Polynomial(v()) - Polynomial(w());
const Polynomial pu = Polynomial(u());
const Polynomial pv = Polynomial(v());
const Polynomial pw = Polynomial(w());

double ts() {
    return static_cast<double>(clock()) / CLOCKS_PER_SEC;
}

void ElementShapeInfo::precompute_functions() {
    std::cerr << ts() << " precompute_functions()" << std::endl;

    /* xyz[0] is a polynomial in terms of (u, v, w), and the x-coordinates of
    the vertices, which computes the x-coordinate of the point at element
    coordinates (u, v, w). Similarly with xyz[1] for the y-coordinate and xyz[2]
    for the z-coordinate. */
    Polynomial xyz[3];
    for (int vertex = 0; vertex < static_cast<int>(vertices.size()); ++vertex) {
        for (int dimension = 0; dimension < 3; ++dimension) {
            xyz[dimension] += Polynomial(coord(vertex, dimension)) *
                vertices[vertex].shape_function;
        }
    }

    std::cerr << ts() << " done with xyz" << std::endl;

    /* jac is the Jacobian matrix for the transformation from (u, v, w)
    coordinates to (x, y, z) coordinates. Each element is a polynomial in (u, v,
    w) and the coordinates of the vertices. */
    Polynomial jac[3][3];
    Polynomial::Variable uvw[3] = {u(), v(), w()};
    jacobian(&xyz, uvw, &jac);

    std::cerr << ts() << " done with jacobian " << jac[0][0].num_terms() << std::endl;

    /* jac_det is the determinant of the Jacobian, a polynomial in (u, v, w) and
    the coordinates of the vertices. It is equal to the ratio between the
    infinitesimal volume element dx*dy*dz and the infinitesimal volume element
    du*dv*dw. */
    Polynomial jac_det = determinant(&jac);

    std::cerr << ts() << " done with jac_det " << jac_det.num_terms() << std::endl;

    /* Integrating the determinant of the Jacobian over the (u, v, w) space
    spanned by the element, gives the volume of the element in (x, y, z)
    coordinates. This is a polynomial in the coordinates of the vertices and not
    in (u, v, w). */
    volume_function = integrate_uvw(jac_det);

    std::cerr << ts() << " done with vf " << volume_function.num_terms() << std::endl;

    /* Integrating (jac_det * shape_function) over the (u, v, w) space gives the
    weighted volume of the part of the element influenced by that vertex. */
    for (int vertex = 0; vertex < static_cast<int>(vertices.size()); ++vertex) {
        vertices[vertex].volume_function = integrate_uvw(
            vertices[vertex].shape_function * jac_det);
        std::cerr << ts() << " done with vfn " << vertices[vertex].volume_function.num_terms() << std::endl;
    }
}

class ElementShapeInfoTetrahedron4 : public ElementShapeInfo {
public:
    ElementShapeInfoTetrahedron4() {
        vertices.resize(4);
        Vertex::Type c = Vertex::Type::Corner;
        vertices[0] = Vertex(c, 0, 0, 0, pt);
        vertices[1] = Vertex(c, 1, 0, 0, pu);
        vertices[2] = Vertex(c, 0, 1, 0, pv);
        vertices[3] = Vertex(c, 0, 0, 1, pw);

        faces.resize(4);
        faces[0].vertices = { 0, 2, 1 };
        faces[1].vertices = { 0, 1, 3 };
        faces[2].vertices = { 1, 2, 3 };
        faces[3].vertices = { 0, 3, 2 };

        precompute_functions();
    }

    Polynomial integrate_uvw(const Polynomial &poly) const {
        return poly
            .integrate(u(), p(0), p(1) - pv - pw)
            .integrate(v(), p(0), p(1) - pw)
            .integrate(w(), p(0), p(1));
    }
};

const ElementShapeInfo &element_shape_tetrahedron4() {
    static const ElementShapeInfoTetrahedron4 info;
    return info;
}

class ElementShapeInfoTetrahedron10 : public ElementShapeInfo {
public:
    ElementShapeInfoTetrahedron10() {
        vertices.resize(10);
        Vertex::Type c = Vertex::Type::Corner;
        Vertex::Type e = Vertex::Type::Edge;
        vertices[0] = Vertex(c, 0.0, 0.0, 0.0, pt * (p(2) * pt - p(1)));
        vertices[1] = Vertex(c, 1.0, 0.0, 0.0, pu * (p(2) * pu - p(1)));
        vertices[2] = Vertex(c, 0.0, 1.0, 0.0, pv * (p(2) * pv - p(1)));
        vertices[3] = Vertex(c, 0.0, 0.0, 1.0, pw * (p(2) * pw - p(1)));
        vertices[4] = Vertex(e, 0.5, 0.0, 0.0, p(4) * pt * pu);
        vertices[5] = Vertex(e, 0.5, 0.5, 0.0, p(4) * pu * pv);
        vertices[6] = Vertex(e, 0.0, 0.5, 0.0, p(4) * pt * pv);
        vertices[7] = Vertex(e, 0.0, 0.0, 0.5, p(4) * pt * pw);
        vertices[8] = Vertex(e, 0.5, 0.0, 0.5, p(4) * pw * pu);
        vertices[9] = Vertex(e, 0.0, 0.5, 0.5, p(4) * pv * pw);

        faces.resize(4);
        faces[0].vertices = { 0, 6, 2, 5, 1, 4 };
        faces[1].vertices = { 0, 4, 1, 8, 3, 7 };
        faces[2].vertices = { 1, 5, 2, 9, 3, 8 };
        faces[3].vertices = { 0, 7, 3, 9, 2, 6 };

        precompute_functions();
    }

    Polynomial integrate_uvw(const Polynomial &poly) const {
        return poly
            .integrate(u(), p(0), p(1) - pv - pw)
            .integrate(v(), p(0), p(1) - pw)
            .integrate(w(), p(0), p(1));
    }
};

const ElementShapeInfo &element_shape_tetrahedron10() {
    static const ElementShapeInfoTetrahedron10 info;
    return info;
}

bool valid_element_type(ElementType type) {
    switch (type) {
    case ElementType::C3D4:
        return true;
    case ElementType::C3D10:
        return true;
    default:
        return false;
    }
}

ElementType element_type_from_string(const std::string &str) {
    if (str == "C3D4") return ElementType::C3D4;
    if (str == "C3D10") return ElementType::C3D10;
    throw std::domain_error("no such element type: " + str);
}

const ElementTypeInfo &ElementTypeInfo::get(ElementType type) {
    switch (type) {
    case ElementType::C3D4: return element_type_c3d4();
    case ElementType::C3D10: return element_type_c3d10();
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

const ElementTypeInfo &element_type_c3d10() {
    static ElementTypeInfo info {
        ElementType::C3D10,
        "C3D10",
        &element_shape_tetrahedron10()
    };
    return info;
}

void element_shape_precompute_functions() {
    /* Trigger initialization of static variables */
    element_type_c3d4();
    element_type_c3d10();
}

} /* namespace os2cx */

