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
    for (int vertex = 0; vertex < static_cast<int>(vertices.size()); ++vertex) {
        for (int dimension = 0; dimension < 3; ++dimension) {
            shape_xyz[dimension] += Polynomial(coord(vertex, dimension)) *
                vertices[vertex].shape_function;
        }
    }

    for (int dimension = 0; dimension < 3; ++dimension) {
        jacobian[dimension][0] = shape_xyz[dimension].differentiate(u());
        jacobian[dimension][1] = shape_xyz[dimension].differentiate(v());
        jacobian[dimension][2] = shape_xyz[dimension].differentiate(w());
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

        integration_points.resize(1);
        integration_points[0] = IntegrationPoint(0.25, 0.25, 0.25, 1/6.0);

        precompute_functions();
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

        integration_points.resize(4);
        double a = (5 - sqrt(5)) / 20, b = (5 + 3 * sqrt(5)) / 20;
        integration_points[0] = IntegrationPoint(a, a, a, 1/24.0);
        integration_points[1] = IntegrationPoint(b, a, a, 1/24.0);
        integration_points[2] = IntegrationPoint(a, b, a, 1/24.0);
        integration_points[3] = IntegrationPoint(a, a, b, 1/24.0);

        precompute_functions();
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

} /* namespace os2cx */

