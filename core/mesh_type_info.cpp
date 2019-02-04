#include "mesh_type_info.hpp"

#include <assert.h>

#include <set>
#include <stdexcept>

namespace os2cx {

static std::set<std::string> all_element_types_including_unsupported({
    "C3D8", "F3D8", "C3D8R", "F3D8R", "C3D8I",
    "C3D20", "F3D20", "C3D20R", "F3D20R", "C3D20RI",
    "C3D8", "F3D8", "C3D10", "F3D10",
    "C3D6", "F3D6", "C3D15", "F3D15",
    "S3", "S4", "S4R", "S6", "S8", "S8R",
    "CPS3", "CPS4", "CPS4R", "CPS8", "CPS8R",
    "CPE3", "CPE4", "CPE4R", "CPE6", "CPE8", "CPE8R",
    "CAX3", "CAX4", "CAX4R", "CAX6", "CAX8", "CAX8R",
    "B31", "B31R", "B32", "B32R",
    "D", "GAPUNI", "DASHPOTA", "SPRINGA", "DCOUP3D"
});

ElementType element_type_from_string(const std::string &str) {
    if (str == "C3D8") return ElementType::C3D8;
    if (str == "C3D20") return ElementType::C3D20;
    if (str == "C3D20R") return ElementType::C3D20R;
    if (str == "C3D20RI") return ElementType::C3D20RI;
    if (str == "C3D4") return ElementType::C3D4;
    if (str == "C3D10") return ElementType::C3D10;

    if (all_element_types_including_unsupported.count(str)) {
        throw std::domain_error("OS2CX does not support element type: " + str);
    }

    throw std::domain_error("no such element type: " + str);
}

void ElementTypeShape::precalculate_face_info() {
    for (Face &face : faces) {
        if (face.vertices.size() == 4) {
            /* first-order rectangular face */
            ShapePoint a = vertices[face.vertices[0]].uvw;
            ShapePoint b = vertices[face.vertices[1]].uvw;
            ShapePoint c = vertices[face.vertices[2]].uvw;
            ShapePoint d = vertices[face.vertices[3]].uvw;

            ShapeVector vector = (b - a).cross(d - a);
            double magnitude = vector.magnitude();
            face.normal = vector / magnitude;

            double total_area = magnitude;

            face.integration_points.resize(1);
            face.integration_points[0].uvw = a + (c - a) / 2;
            face.integration_points[0].weight = total_area;

        } else if (face.vertices.size() == 8) {
            /* second-order rectangular face */
            ShapePoint a = vertices[face.vertices[0]].uvw;
            ShapePoint b = vertices[face.vertices[2]].uvw;
            ShapePoint c = vertices[face.vertices[4]].uvw;
            ShapePoint d = vertices[face.vertices[6]].uvw;

            ShapeVector vector = (b - a).cross(d - a);
            double magnitude = vector.magnitude();
            face.normal = vector / magnitude;

            double total_area = magnitude;

            ShapePoint center = a + (c - a) / 2;
            face.integration_points.resize(4);
            face.integration_points[0].uvw = center + (a - center) / sqrt(3);
            face.integration_points[0].weight = total_area / 4;
            face.integration_points[1].uvw = center + (b - center) / sqrt(3);
            face.integration_points[1].weight = total_area / 4;
            face.integration_points[2].uvw = center + (c - center) / sqrt(3);
            face.integration_points[2].weight = total_area / 4;
            face.integration_points[3].uvw = center + (d - center) / sqrt(3);
            face.integration_points[3].weight = total_area / 4;

        } else if (face.vertices.size() == 3) {
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

/* eight_volume_integration_points() defines the integration point scheme that's
shared between C3D8, C3D20R, and C3D20RI. */
std::vector<ElementTypeShape::IntegrationPoint>
    eight_volume_integration_points()
{
    std::vector<ElementTypeShape::IntegrationPoint>
        volume_integration_points(8);
    double locs[2] = {-1/sqrt(3), +1/sqrt(3)};
    for (int i = 0; i < 8; ++i) {
        int x = i % 2, y = (i / 2) % 2, z = i / 4;
        volume_integration_points[i] =
            ElementTypeShape::IntegrationPoint(locs[x], locs[y], locs[z], 1.0);
    }
    return volume_integration_points;
}

class ElementTypeShapeC3D8 : public ElementTypeShape {
public:
    ElementTypeShapeC3D8() {
        name = "C3D8";
        category = Category::Brick;
        order = 1;

        vertices.resize(8);
        static const Vertex::Type c = Vertex::Type::Corner;
        vertices[0] = Vertex(c, -1, -1, -1);
        vertices[1] = Vertex(c, +1, -1, -1);
        vertices[2] = Vertex(c, +1, +1, -1);
        vertices[3] = Vertex(c, -1, +1, -1);
        vertices[4] = Vertex(c, -1, -1, +1);
        vertices[5] = Vertex(c, +1, -1, +1);
        vertices[6] = Vertex(c, +1, +1, +1);
        vertices[7] = Vertex(c, -1, +1, +1);

        faces.resize(6);
        faces[0].vertices = { 0, 3, 2, 1 };
        faces[1].vertices = { 4, 5, 6, 7 };
        faces[2].vertices = { 0, 1, 5, 4 };
        faces[3].vertices = { 1, 2, 6, 5 };
        faces[4].vertices = { 2, 3, 7, 6 };
        faces[5].vertices = { 0, 4, 7, 3 };
        precalculate_face_info();

        volume_integration_points = eight_volume_integration_points();
    }

    void shape_functions(ShapePoint uvw, double *sf_out) const {
        for (int i = 0; i < 8; ++i) {
            double sx = 1 + uvw.x * vertices[i].uvw.x;
            double sy = 1 + uvw.y * vertices[i].uvw.y;
            double sz = 1 + uvw.z * vertices[i].uvw.z;
            sf_out[i] = 0.125 * sx * sy * sz;
        }
    }

    void shape_function_derivatives(
        ShapePoint uvw,
        ShapeVector *sf_d_uvw_out
    ) const {
        for (int i = 0; i < 8; ++i) {
            double sx = 1 + uvw.x * vertices[i].uvw.x;
            double sy = 1 + uvw.y * vertices[i].uvw.y;
            double sz = 1 + uvw.z * vertices[i].uvw.z;
            sf_d_uvw_out[i] = ShapeVector(
                0.125 * vertices[i].uvw.x * sy * sz,
                0.125 * sx * vertices[i].uvw.y * sz,
                0.125 * sx * sy * vertices[i].uvw.z
            );
        }
    }
};

class ElementTypeShapeC3D20Abstract : public ElementTypeShape {
public:
    ElementTypeShapeC3D20Abstract() {
        category = Category::Brick;
        order = 2;

        vertices.resize(20);
        static const Vertex::Type c = Vertex::Type::Corner;
        static const Vertex::Type e = Vertex::Type::Edge;
        vertices[ 0] = Vertex(c, -1, -1, -1);
        vertices[ 1] = Vertex(c, +1, -1, -1);
        vertices[ 2] = Vertex(c, +1, +1, -1);
        vertices[ 3] = Vertex(c, -1, +1, -1);
        vertices[ 4] = Vertex(c, -1, -1, +1);
        vertices[ 5] = Vertex(c, +1, -1, +1);
        vertices[ 6] = Vertex(c, +1, +1, +1);
        vertices[ 7] = Vertex(c, -1, +1, +1);
        vertices[ 8] = Vertex(e,  0, -1, -1);
        vertices[ 9] = Vertex(e, +1,  0, -1);
        vertices[10] = Vertex(e,  0, +1, -1);
        vertices[11] = Vertex(e, -1,  0, -1);
        vertices[12] = Vertex(e,  0, -1, +1);
        vertices[13] = Vertex(e, +1,  0, +1);
        vertices[14] = Vertex(e,  0, +1, +1);
        vertices[15] = Vertex(e, -1,  0, +1);
        vertices[16] = Vertex(c, -1, -1,  0);
        vertices[17] = Vertex(c, +1, -1,  0);
        vertices[18] = Vertex(c, +1, +1,  0);
        vertices[19] = Vertex(c, -1, +1,  0);

        faces.resize(6);
        faces[0].vertices = {  0, 11,  3, 10,  2,  9,  1,  8 };
        faces[1].vertices = {  4, 12,  5, 13,  6, 14,  7, 15 };
        faces[2].vertices = {  0,  8,  1, 17,  5, 12,  4, 16 };
        faces[3].vertices = {  1,  9,  2, 18,  6, 13,  5, 17 };
        faces[4].vertices = {  2, 10,  3, 19,  7, 14,  6, 18 };
        faces[5].vertices = {  0, 16,  4, 15,  7, 19,  3, 11 };
        precalculate_face_info();
    }

    void shape_functions(ShapePoint uvw, double *sf_out) const {
        for (int i = 0; i < 8; ++i) {
            double sx = 1 + uvw.x * vertices[i].uvw.x;
            double sy = 1 + uvw.y * vertices[i].uvw.y;
            double sz = 1 + uvw.z * vertices[i].uvw.z;
            double ss = uvw.x * vertices[i].uvw.x
                      + uvw.y * vertices[i].uvw.y
                      + uvw.z * vertices[i].uvw.z
                     - 2;
            sf_out[i] = 0.125 * sx * sy * sz * ss;
        }
        for (int i : {8, 10, 12, 14}) {
            double sx = 1 - uvw.x * uvw.x;
            double sy = 1 + uvw.y * vertices[i].uvw.y;
            double sz = 1 + uvw.z * vertices[i].uvw.z;
            sf_out[i] = 0.25 * sx * sy * sz;
        }
        for (int i : {9, 11, 13, 15}) {
            double sx = 1 + uvw.x * vertices[i].uvw.x;
            double sy = 1 - uvw.y * uvw.y;
            double sz = 1 + uvw.z * vertices[i].uvw.z;
            sf_out[i] = 0.25 * sx * sy * sz;
        }
        for (int i : {16, 17, 18, 19}) {
            double sx = 1 + uvw.x * vertices[i].uvw.x;
            double sy = 1 + uvw.y * vertices[i].uvw.y;
            double sz = 1 - uvw.z * uvw.z;
            sf_out[i] = 0.25 * sx * sy * sz;
        }
    }

    void shape_function_derivatives(
        ShapePoint uvw,
        ShapeVector *sf_d_uvw_out
    ) const {
        for (int i = 0; i < 8; ++i) {
            double sx = 1 + uvw.x * vertices[i].uvw.x;
            double sy = 1 + uvw.y * vertices[i].uvw.y;
            double sz = 1 + uvw.z * vertices[i].uvw.z;
            double ss = uvw.x * vertices[i].uvw.x
                      + uvw.y * vertices[i].uvw.y
                      + uvw.z * vertices[i].uvw.z
                     - 2;
            sf_d_uvw_out[i] = ShapeVector(
                0.125 * vertices[i].uvw.x * sy * sz * ss
                    + 0.125 * sx * sy * sz * vertices[i].uvw.x,
                0.125 * sx * vertices[i].uvw.y * sz * ss
                    + 0.125 * sx * sy * sz * vertices[i].uvw.y,
                0.125 * sx * sy * vertices[i].uvw.z * ss
                    + 0.125 * sx * sy * sz * vertices[i].uvw.z);
        }
        for (int i : {8, 10, 12, 14}) {
            double sx = 1 - uvw.x * uvw.x;
            double sy = 1 + uvw.y * vertices[i].uvw.y;
            double sz = 1 + uvw.z * vertices[i].uvw.z;
            sf_d_uvw_out[i] = ShapeVector(
                0.25 * (-2 * uvw.x) * sy * sz,
                0.25 * sx * vertices[i].uvw.y * sz,
                0.25 * sx * sy * vertices[i].uvw.z);
        }
        for (int i : {9, 11, 13, 15}) {
            double sx = 1 + uvw.x * vertices[i].uvw.x;
            double sy = 1 - uvw.y * uvw.y;
            double sz = 1 + uvw.z * vertices[i].uvw.z;
            sf_d_uvw_out[i] = ShapeVector(
                0.25 * vertices[i].uvw.x * sy * sz,
                0.25 * sx * (-2 * uvw.y) * sz,
                0.25 * sx * sy * vertices[i].uvw.z);
        }
        for (int i : {16, 17, 18, 19}) {
            double sx = 1 + uvw.x * vertices[i].uvw.x;
            double sy = 1 + uvw.y * vertices[i].uvw.y;
            double sz = 1 - uvw.z * uvw.z;
            sf_d_uvw_out[i] = ShapeVector(
                0.25 * vertices[i].uvw.x * sy * sz,
                0.25 * sx * vertices[i].uvw.y * sz,
                0.25 * sx * sy * (-2 * uvw.z));
        }
    }
};

class ElementTypeShapeC3D20 : public ElementTypeShapeC3D20Abstract {
public:
    ElementTypeShapeC3D20() {
        name = "C3D20";

        volume_integration_points.resize(27);
        double weights[3] = {5/9.0, 8/9.0, 5/9.0};
        double locs[3] = {-sqrt(0.6), 0, +sqrt(0.6)};
        for (int i = 0; i < 27; ++i) {
            int x = i % 3, y = (i / 3) % 3, z = i / 9;
            volume_integration_points[i] = IntegrationPoint(
                locs[x], locs[y], locs[z],
                weights[x] * weights[y] * weights[z]);
        }
    }
};

class ElementTypeShapeC3D20R : public ElementTypeShapeC3D20Abstract {
public:
    ElementTypeShapeC3D20R() {
        name = "C3D20R";

        volume_integration_points = eight_volume_integration_points();
    }
};

class ElementTypeShapeC3D20RI : public ElementTypeShapeC3D20Abstract {
public:
    ElementTypeShapeC3D20RI() {
        name = "C3D20RI";

        volume_integration_points = eight_volume_integration_points();
    }
};

class ElementTypeShapeC3D4 : public ElementTypeShape {
public:
    ElementTypeShapeC3D4() {
        name = "C3D4";
        category = Category::Tetrahedron;
        order = 1;

        vertices.resize(4);
        static const Vertex::Type c = Vertex::Type::Corner;
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

class ElementTypeShapeC3D10 : public ElementTypeShape {
public:
    ElementTypeShapeC3D10() {
        name = "C3D10";
        category = Category::Tetrahedron;
        order = 2;

        vertices.resize(10);
        static const Vertex::Type c = Vertex::Type::Corner;
        static const Vertex::Type e = Vertex::Type::Edge;
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

const ElementTypeShape &element_type_shape(ElementType type) {
    static const ElementTypeShapeC3D8    c3d8;
    static const ElementTypeShapeC3D20   c3d20;
    static const ElementTypeShapeC3D20R  c3d20r;
    static const ElementTypeShapeC3D20RI c3d20ri;
    static const ElementTypeShapeC3D4    c3d4;
    static const ElementTypeShapeC3D10   c3d10;

    switch (type) {
    case ElementType::C3D8:    return c3d8;
    case ElementType::C3D20:   return c3d20;
    case ElementType::C3D20R:  return c3d20r;
    case ElementType::C3D20RI: return c3d20ri;
    case ElementType::C3D4:    return c3d4;
    case ElementType::C3D10:   return c3d10;
    default: assert(false);
    }
}

} /* namespace os2cx */

