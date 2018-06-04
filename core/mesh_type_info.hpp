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

inline Polynomial::Variable u() { return Polynomial::Variable { -1 }; }
inline Polynomial::Variable v() { return Polynomial::Variable { -2 }; }
inline Polynomial::Variable w() { return Polynomial::Variable { -3 }; }

inline Polynomial::Variable coord(int vertex, int dimension) {
    return Polynomial::Variable { vertex << 2 | dimension };
}
inline int coord_to_vertex(Polynomial::Variable var) {
    return var.index >> 2;
}
inline int coord_to_dimension(Polynomial::Variable var) {
    return var.index & 3;
}

inline Polynomial p(Polynomial::Variable v) { return Polynomial(v); }
inline Polynomial p(double v) { return Polynomial(v); }

} /* namespace shape_function_variables */

class ElementShapeInfo {
public:
    class Vertex {
    public:
        enum class Type { Corner, Edge };

        Vertex() { }
        Vertex(Type t, double su, double sv, double sw, Polynomial sf) :
            type(t), shape_u(su), shape_v(sv), shape_w(sw), shape_function(sf)
        { }

        Type type;
        double shape_u, shape_v, shape_w;
        Polynomial shape_function;
        Polynomial volume_function;
    };
    class Face {
    public:
        /* vertex indices in counterclockwise order looking from outside */
        std::vector<int> vertices;
    };

    static const int max_faces_per_element = 6;
    static const int max_vertices_per_face = 8;
    static const int max_vertices_per_element = 20;

    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    Polynomial volume_function;

protected:
    /* Subclass constructor must set up everything except for
    Vertex::volume_function and ElementShapeInfo::volume_function, then call
    precompute_functions() to calculate those */
    void precompute_functions();

    /* Given a polynomial in terms of u, v, w, and possibly some other
    variables, integrates u, v, and w over the volume of the shape. */
    virtual Polynomial integrate_uvw(const Polynomial &poly) const = 0;
};

const ElementShapeInfo &element_shape_tetrahedron4();

enum class ElementType {
    C3D4 = 3,
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

} /* namespace os2cx */

#endif

