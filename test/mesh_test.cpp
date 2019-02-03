#include <gtest/gtest.h>

#include "mesh.hpp"

namespace os2cx {

void check_element_type_shape_functions(
    const ElementTypeShape &shape,
    ElementTypeShape::ShapePoint uvw
) {
    double sf[ElementTypeShape::max_vertices_per_element];
    shape.shape_functions(uvw, sf);

    /* Sanity check that shape functions sum to 1 */
    double total = 0;
    for (int i = 0; i < static_cast<int>(shape.vertices.size()); ++i) {
        total += sf[i];
    }
    EXPECT_FLOAT_EQ(1, total);

    /* Sanity check that ElementShapeInfo.shape_function_derivatives is actually
    correctly computing the derivatives of ElementShapeInfo.shape_function,
    because it's easy to make mistakes when doing many derivatives by hand. */
    ElementTypeShape::ShapeVector sf_d_uvw[
        ElementTypeShape::max_vertices_per_element];
    shape.shape_function_derivatives(uvw, sf_d_uvw);
    static const double epsilon = 1e-6;
    for (int i = 0; i < 3; ++i) {
        ElementTypeShape::ShapeVector delta_uvw(
            i == 0 ? epsilon : 0,
            i == 1 ? epsilon : 0,
            i == 2 ? epsilon : 0);
        ElementTypeShape::ShapePoint uvw_prime = uvw + delta_uvw;
        double sf_prime[ElementTypeShape::max_vertices_per_element];
        shape.shape_functions(uvw_prime, sf_prime);
        for (int j = 0; j < static_cast<int>(shape.vertices.size());
                ++j) {
            double extrapolated = sf[j] + sf_d_uvw[j].dot(delta_uvw);
            double actual = sf_prime[j];
            /* extrapolation should be correct to within O(epsilon^2) */
            EXPECT_NEAR(actual, extrapolated, 10 * epsilon * epsilon) <<
                "error in derivative of sf[" << j << "] " <<
                "with respect to uvw[" << i << "]";
        }
    }
}

void check_element_type_shape(const ElementTypeShape &shape) {
    /* arbitrary test point */
    ElementTypeShape::ShapePoint uvw(0.11, 0.22, 0.33);
    check_element_type_shape_functions(shape, uvw);
}

TEST(MeshTest, CheckElementTypeShapeC3D8) {
    check_element_type_shape(element_type_shape_c3d8());
}

TEST(MeshTest, CheckElementTypeShapeC3D4) {
    check_element_type_shape(element_type_shape_c3d4());
}

TEST(MeshTest, CheckElementTypeShapeC3D10) {
    check_element_type_shape(element_type_shape_c3d10());
}

Mesh3 make_example_mesh(ElementType type, AffineTransform transform) {
    Mesh3 mesh;
    const ElementTypeShape &shape = element_type_shape(type);
    Element3 element;
    element.type = type;
    for (int i = 0; i < static_cast<int>(shape.vertices.size()); ++i) {
        Node3 node;
        node.point = transform.apply(shape.vertices[i].uvw);
        element.nodes[i] = mesh.nodes.key_end();
        mesh.nodes.push_back(node);
    }
    mesh.elements.push_back(element);
    return mesh;
}

TEST(MeshTest, Volume) {
    Mesh3 mesh1 = make_example_mesh(
        ElementType::C3D4,
        AffineTransform(Matrix::identity()));
    EXPECT_FLOAT_EQ(1/6.0, mesh1.volume(*mesh1.elements.begin()));

    Mesh3 mesh2 = make_example_mesh(
        ElementType::C3D10,
        AffineTransform(Matrix::identity()));
    EXPECT_FLOAT_EQ(1/6.0, mesh2.volume(*mesh2.elements.begin()));

    Mesh3 mesh3 = make_example_mesh(
        ElementType::C3D10,
        AffineTransform(Matrix::scale(3, 4, 5)));
    EXPECT_FLOAT_EQ(1/6.0*60, mesh3.volume(*mesh3.elements.begin()));

    Mesh3 mesh4 = make_example_mesh(
        ElementType::C3D10,
        AffineTransform(Matrix::rotation(0, 1.23)));
    EXPECT_FLOAT_EQ(1/6.0, mesh4.volume(*mesh4.elements.begin()));
}

TEST(MeshTest, Area) {
    Mesh3 mesh1 = make_example_mesh(
        ElementType::C3D4,
        AffineTransform(Matrix::identity()));
    Vector area1 = mesh1.oriented_area(*mesh1.elements.begin(), 0);
    EXPECT_FLOAT_EQ(0, area1.x);
    EXPECT_FLOAT_EQ(0, area1.y);
    EXPECT_FLOAT_EQ(-1/2.0, area1.z);

    Mesh3 mesh2 = make_example_mesh(
        ElementType::C3D10,
        AffineTransform(Matrix::identity()));
    Vector area2 = mesh2.oriented_area(*mesh2.elements.begin(), 0);
    EXPECT_FLOAT_EQ(0, area2.x);
    EXPECT_FLOAT_EQ(0, area2.y);
    EXPECT_FLOAT_EQ(-1/2.0, area2.z);

    Mesh3 mesh3 = make_example_mesh(
        ElementType::C3D10,
        AffineTransform(Matrix::scale(3, 4, 5)));
    Vector area3 = mesh3.oriented_area(*mesh3.elements.begin(), 0);
    EXPECT_FLOAT_EQ(0, area3.x);
    EXPECT_FLOAT_EQ(0, area3.y);
    EXPECT_FLOAT_EQ(-1/2.0*12, area3.z);

    Mesh3 mesh4 = make_example_mesh(
        ElementType::C3D10,
        AffineTransform(Matrix::rotation(0, 1.23)));
    Vector area4 = mesh4.oriented_area(*mesh4.elements.begin(), 0);
    EXPECT_FLOAT_EQ(0, area4.x);
    EXPECT_FLOAT_EQ(-1/2.0*-sin(1.23), area4.y);
    EXPECT_FLOAT_EQ(-1/2.0*cos(1.23), area4.z);
}

} /* namespace os2cx */
