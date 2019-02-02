#ifndef OS2CX_MESH_TYPE_INFO_HPP_
#define OS2CX_MESH_TYPE_INFO_HPP_

#include <string>
#include <vector>

#include "calc.hpp"

namespace os2cx {

class ElementShapeInfo {
public:
    static const int max_faces_per_element = 6;
    static const int max_vertices_per_face = 8;
    static const int max_vertices_per_element = 20;

    /* ShapeVector and ShapePoint represent vectors and points in the shape's
    local (U, V, W) coordinate system, as opposed to the global (X, Y, Z)
    coordinate system. But it's still the same underlying type, so you have to
    write "shape_vector.x" when you want the U-coordinate, and so on. */
    typedef Vector ShapeVector;
    typedef Point ShapePoint;

    class IntegrationPoint {
    public:
        IntegrationPoint() { }
        IntegrationPoint(double u, double v, double w, double we) :
            uvw(u, v, w), weight(we) { }
        ShapePoint uvw;
        double weight;
    };

    class Vertex {
    public:
        enum class Type { Corner, Edge };

        Vertex() { }
        Vertex(Type ty, double u, double v, double w) :
            type(ty), uvw(u, v, w)
        { }

        Type type;
        ShapePoint uvw;
    };
    std::vector<Vertex> vertices;

    class Face {
    public:
        /* vertex indices in counterclockwise order looking from outside */
        std::vector<int> vertices;

        /* normalized vector pointing outwards */
        ShapeVector normal;

        /* integration points for surface integrals */
        std::vector<IntegrationPoint> integration_points;
    };
    std::vector<Face> faces;

    /* integration points for volume integrals */
    std::vector<IntegrationPoint> volume_integration_points;

    virtual void shape_functions(
        ShapePoint uvw,
        /* sf_out[i] will be the shape function for vertex 'i' */
        double *sf_out
    ) const = 0;

    virtual void shape_function_derivatives(
        ShapePoint uvw,
        /* sf_d_uvw_out[i].x will be the derivative of the shape function for
        vertex 'i' with respect to 'u'. Similarly, '.y' is the derivative with
        respect to 'v', and '.z' is the derivative with respect to 'w'. */
        ShapeVector *sf_d_uvw_out
    ) const = 0;

protected:
    /* Computes faces[*].normal and faces[*].integration_points from
    faces[*].vertices and vertices[*].uvw */
    void precalculate_face_info();
};

const ElementShapeInfo &element_shape_brick8();
const ElementShapeInfo &element_shape_tetrahedron4();
const ElementShapeInfo &element_shape_tetrahedron10();

enum class ElementType {
    /* Numeric values match those used by CalculiX GraphiX .FRD files */
    C3D8 = 1,
    C3D4 = 3,
    C3D10 = 6,
};

bool valid_element_type(ElementType);
ElementType element_type_from_string(const std::string &str);

class ElementTypeInfo {
public:
    static const ElementTypeInfo &get(ElementType type);

    ElementType type;
    std::string name;
    const ElementShapeInfo *shape;
};

const ElementTypeInfo &element_type_c3d8();
const ElementTypeInfo &element_type_c3d4();
const ElementTypeInfo &element_type_c3d10();

} /* namespace os2cx */

#endif

