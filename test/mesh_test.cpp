#include <gtest/gtest.h>

#include "mesh.hpp"

namespace os2cx {


void check_element_shape_info_shape_functions(
    const ElementShapeInfo &shape,
    ElementShapeInfo::ShapePoint uvw
) {
    double sf[ElementShapeInfo::max_vertices_per_element];
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
    ElementShapeInfo::ShapeVector sf_d_uvw[
        ElementShapeInfo::max_vertices_per_element];
    shape.shape_function_derivatives(uvw, sf_d_uvw);
    static const double epsilon = 1e-6;
    for (int i = 0; i < 3; ++i) {
        ElementShapeInfo::ShapeVector delta_uvw(
            i == 0 ? epsilon : 0,
            i == 1 ? epsilon : 0,
            i == 2 ? epsilon : 0);
        ElementShapeInfo::ShapePoint uvw_prime = uvw + delta_uvw;
        double sf_prime[ElementShapeInfo::max_vertices_per_element];
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

void check_element_shape_info(const ElementShapeInfo &shape) {
    ElementShapeInfo::ShapePoint uvw(0.11, 0.22, 0.33); /* arbitrary test point */
    check_element_shape_info_shape_functions(shape, uvw);
}

TEST(MeshTest, CheckTetrahedron4) {
    check_element_shape_info(element_shape_tetrahedron4());
}

TEST(MeshTest, CheckTetrahedron10) {
    check_element_shape_info(element_shape_tetrahedron10());
}

} /* namespace os2cx */
