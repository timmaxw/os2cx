#ifndef OS2CX_MESH_TYPE_INFO_HPP_
#define OS2CX_MESH_TYPE_INFO_HPP_

#include <string>
#include <vector>

#include "calc.hpp"
#include "polynomial.hpp"

namespace os2cx {

/* Shape functions are defined in terms of three variables u, v, and w. Volume
functions are defined in terms of the x, y, and z coordinates of the vertices.
shape_function_variables contains helper functions for working with these
variables, bundled into a namespace so that they can have short names without
polluting the os2cx namespace. */

namespace shape_function_variables {

inline Polynomial::Variable coord(int vertex, int dimension) {
    return Polynomial::Variable::from_index(vertex << 2 | dimension);
}
inline int coord_to_vertex(Polynomial::Variable var) {
    return var.index >> 2;
}
inline int coord_to_dimension(Polynomial::Variable var) {
    return var.index & 3;
}

inline Polynomial::Variable u() { return Polynomial::Variable { 20 * 4 + 0 }; }
inline Polynomial::Variable v() { return Polynomial::Variable { 20 * 4 + 1 }; }
inline Polynomial::Variable w() { return Polynomial::Variable { 20 * 4 + 2 }; }

constexpr int max_var_plus_one = 20 * 4 + 3;

} /* namespace shape_function_variables */

class ElementShapeInfo {
public:
    static const int max_faces_per_element = 6;
    static const int max_vertices_per_face = 8;
    static const int max_vertices_per_element = 20;

    class Vertex {
    public:
        enum class Type { Corner, Edge };

        Vertex() { }
        Vertex(Type ty, double su, double sv, double sw, Polynomial sf) :
            type(ty), shape_uvw { su, sv, sw }, shape_function(sf)
        { }

        Type type;
        double shape_uvw[3];
        Polynomial shape_function;
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

    Polynomial shape_xyz[3];
    Polynomial jacobian[3][3];

protected:
    /* Subclass constructor must set up everything except for 'shape_{x,y,z}'
    and 'jacobian', then call precompute_functions() to calculate those */
    void precompute_functions();
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

