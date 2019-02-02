#include "mesh_type_info.hpp"

#include <assert.h>

#include <stdexcept>

namespace os2cx {

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

void ElementTypeShape::precalculate_face_info() {
    for (Face &face : faces) {
        if (face.vertices.size() == 3) {
            /* first-order triangular face */
            ShapePoint a = vertices[face.vertices[0]].uvw;
            ShapePoint b = vertices[face.vertices[1]].uvw;
            ShapePoint c = vertices[face.vertices[2]].uvw;

            ShapeVector vector = (b - a).cross(c - a);
            double magnitude = vector.magnitude();
            face.normal = vector / magnitude;

            double total_area = magnitude / 2;

            face.integration_points.resize(1);
            face.integration_points[0].uvw =  a + (b - a) / 3 + (c - a) / 3;
            face.integration_points[0].weight = total_area;

        } else if (face.vertices.size() == 6) {
            /* second-order triangular face */
            ShapePoint a = vertices[face.vertices[0]].uvw;
            ShapePoint b = vertices[face.vertices[2]].uvw;
            ShapePoint c = vertices[face.vertices[4]].uvw;

            ShapeVector vector = (b - a).cross(c - a);
            double magnitude = vector.magnitude();
            face.normal = vector / magnitude;

            double total_area = magnitude / 2;

            face.integration_points.resize(3);
            face.integration_points[0].uvw =  a + (b - a) / 6 + (c - a) / 6;
            face.integration_points[0].weight = total_area / 3;
            face.integration_points[1].uvw =  a + (b - a) * 2 / 3 + (c - a) / 6;
            face.integration_points[1].weight = total_area / 3;
            face.integration_points[2].uvw =  a + (b - a) / 6 + (c - a) * 2 / 3;
            face.integration_points[2].weight = total_area / 3;

        } else {
            assert(false);
        }
    }
}

class ElementTypeShapeC3D4 : public ElementTypeShape {
public:
    ElementTypeShapeC3D4() {
        name = "C3D4";

        vertices.resize(4);
        Vertex::Type c = Vertex::Type::Corner;
        vertices[0] = Vertex(c, 0, 0, 0);
        vertices[1] = Vertex(c, 1, 0, 0);
        vertices[2] = Vertex(c, 0, 1, 0);
        vertices[3] = Vertex(c, 0, 0, 1);

        faces.resize(4);
        faces[0].vertices = { 0, 2, 1 };
        faces[1].vertices = { 0, 1, 3 };
        faces[2].vertices = { 1, 2, 3 };
        faces[3].vertices = { 0, 3, 2 };
        precalculate_face_info();

        volume_integration_points.resize(1);
        volume_integration_points[0] = IntegrationPoint(0.25, 0.25, 0.25, 1/6.0);
    }

    void shape_functions(ShapePoint uvw, double *sf_out) const {
        sf_out[0] = 1.0 - uvw.x - uvw.y - uvw.z;
        sf_out[1] = uvw.x;
        sf_out[2] = uvw.y;
        sf_out[3] = uvw.z;
    }

    void shape_function_derivatives(
        ShapePoint uvw,
        ShapeVector *sf_d_uvw_out
    ) const {
        (void)uvw;
        sf_d_uvw_out[0] = ShapeVector(-1, -1, -1);
        sf_d_uvw_out[1] = ShapeVector( 1,  0,  0);
        sf_d_uvw_out[2] = ShapeVector( 0,  1,  0);
        sf_d_uvw_out[3] = ShapeVector( 0,  0,  1);
    }
};

const ElementTypeShape &element_type_shape_c3d4() {
    static const ElementTypeShapeC3D4 shape;
    return shape;
}

class ElementTypeShapeC3D10 : public ElementTypeShape {
public:
    ElementTypeShapeC3D10() {
        name = "C3D10";

        vertices.resize(10);
        Vertex::Type c = Vertex::Type::Corner;
        Vertex::Type e = Vertex::Type::Edge;
        vertices[0] = Vertex(c, 0.0, 0.0, 0.0);
        vertices[1] = Vertex(c, 1.0, 0.0, 0.0);
        vertices[2] = Vertex(c, 0.0, 1.0, 0.0);
        vertices[3] = Vertex(c, 0.0, 0.0, 1.0);
        vertices[4] = Vertex(e, 0.5, 0.0, 0.0);
        vertices[5] = Vertex(e, 0.5, 0.5, 0.0);
        vertices[6] = Vertex(e, 0.0, 0.5, 0.0);
        vertices[7] = Vertex(e, 0.0, 0.0, 0.5);
        vertices[8] = Vertex(e, 0.5, 0.0, 0.5);
        vertices[9] = Vertex(e, 0.0, 0.5, 0.5);

        faces.resize(4);
        faces[0].vertices = { 0, 6, 2, 5, 1, 4 };
        faces[1].vertices = { 0, 4, 1, 8, 3, 7 };
        faces[2].vertices = { 1, 5, 2, 9, 3, 8 };
        faces[3].vertices = { 0, 7, 3, 9, 2, 6 };
        precalculate_face_info();

        volume_integration_points.resize(4);
        double a = (5 - sqrt(5)) / 20, b = (5 + 3 * sqrt(5)) / 20;
        volume_integration_points[0] = IntegrationPoint(a, a, a, 1/24.0);
        volume_integration_points[1] = IntegrationPoint(b, a, a, 1/24.0);
        volume_integration_points[2] = IntegrationPoint(a, b, a, 1/24.0);
        volume_integration_points[3] = IntegrationPoint(a, a, b, 1/24.0);
    }

    void shape_functions(ShapePoint uvw, double *sf_out) const {
        double u = uvw.x, v = uvw.y, w = uvw.z;
        double t = 1.0 - u - v - w;
        sf_out[0] = t * (2 * t - 1);
        sf_out[1] = u * (2 * u - 1);
        sf_out[2] = v * (2 * v - 1);
        sf_out[3] = w * (2 * w - 1);
        sf_out[4] = 4 * t * u;
        sf_out[5] = 4 * u * v;
        sf_out[6] = 4 * t * v;
        sf_out[7] = 4 * t * w;
        sf_out[8] = 4 * w * u;
        sf_out[9] = 4 * v * w;
    }

    void shape_function_derivatives(
        ShapePoint uvw,
        ShapeVector *sf_d_uvw_out
    ) const {
        double u = uvw.x, v = uvw.y, w = uvw.z;
        double t = 1.0 - u - v - w;

        sf_d_uvw_out[0] = ShapeVector(  -4*t+1,   -4*t+1,   -4*t+1);
        sf_d_uvw_out[1] = ShapeVector(   4*u-1,        0,        0);
        sf_d_uvw_out[2] = ShapeVector(       0,    4*v-1,        0);
        sf_d_uvw_out[3] = ShapeVector(       0,        0,    4*w-1);
        sf_d_uvw_out[4] = ShapeVector(-4*u+4*t,     -4*u,     -4*u);
        sf_d_uvw_out[5] = ShapeVector(     4*v,      4*u,        0);
        sf_d_uvw_out[6] = ShapeVector(    -4*v, -4*v+4*t,     -4*v);
        sf_d_uvw_out[7] = ShapeVector(    -4*w,     -4*w, -4*w+4*t);
        sf_d_uvw_out[8] = ShapeVector(     4*w,        0,      4*u);
        sf_d_uvw_out[9] = ShapeVector(       0,      4*w,      4*v);
    }
};

const ElementTypeShape &element_type_shape_c3d10() {
    static const ElementTypeShapeC3D10 shape;
    return shape;
}


const ElementTypeShape &element_type_shape(ElementType type) {
    switch (type) {
    case ElementType::C3D4: return element_type_shape_c3d4();
    case ElementType::C3D10: return element_type_shape_c3d10();
    default: assert(false);
    }
}

} /* namespace os2cx */

