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

    class Vertex {
    public:
        enum class Type { Corner, Edge };

        Vertex() { }
        Vertex(Type ty, double su, double sv, double sw) :
            type(ty), shape_uvw { su, sv, sw }
        { }

        Type type;
        double shape_uvw[3];
    };
    std::vector<Vertex> vertices;

    class Face {
    public:
        /* vertex indices in counterclockwise order looking from outside */
        std::vector<int> vertices;
    };
    std::vector<Face> faces;

    class IntegrationPoint {
    public:
        IntegrationPoint() { }
        IntegrationPoint(double su, double sv, double sw, double w) :
            shape_uvw {su, sv, sw}, weight(w) { }
        double shape_uvw[3];
        double weight;
    };
    std::vector<IntegrationPoint> integration_points;

    virtual void shape_functions(
        const double *uvw,
        /* sf_out[i] will be the shape function for vertex 'i' */
        double *sf_out
    ) const = 0;
    virtual void shape_function_derivatives(
        const double *uvw,
        /* sf_d_uvw_out[i*3+j] will be the derivative of the shape function for
        vertex 'i' with respect to 'u' (if j=0), 'v' (if j=1), or 'w' (if j=2).
        */
        double *sf_d_uvw_out
    ) const = 0;
};

const ElementShapeInfo &element_shape_tetrahedron4();
const ElementShapeInfo &element_shape_tetrahedron10();

enum class ElementType {
    C3D4 = 3,
    C3D10 = 6
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

const ElementTypeInfo &element_type_c3d4();
const ElementTypeInfo &element_type_c3d10();

} /* namespace os2cx */

#endif

