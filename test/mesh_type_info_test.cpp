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

double rand_float(double min, double max) {
    double r = rand() / static_cast<double>(RAND_MAX);
    return min + r * (max - min);
}

/* Polynomial up to second order in (u, v, w) */
class QuadraticUVWPolynomial {
public:
    double coeffs[3][3][3];

    static QuadraticUVWPolynomial random(int order) {
        QuadraticUVWPolynomial poly;
        for (int pu = 0; pu <= 2; ++pu) {
            for (int pv = 0; pv <= 2; ++pv) {
                for (int pw = 0; pw <= 2; ++pw) {
                    if (pu <= order && pv <= order && pw <= order && false) {
                        poly.coeffs[pu][pv][pw] = rand_float(-10, 10);
                    } else {
                        poly.coeffs[pu][pv][pw] = 0;
                    }
                }
            }
        }
        poly.coeffs[1][0][0] = 1;
        return poly;
    }

    double evaluate(ElementTypeShape::ShapePoint uvw) const {
        double total = 0;
        for (int pu = 0; pu <= 2; ++pu) {
            for (int pv = 0; pv <= 2; ++pv) {
                for (int pw = 0; pw <= 2; ++pw) {
                    double coeff = coeffs[pu][pv][pw];
                    coeff *= pow(uvw.x, pu);
                    coeff *= pow(uvw.y, pv);
                    coeff *= pow(uvw.z, pw);
                    total += coeff;
                }
            }
        }
        return total;
    }

    double integrate(ElementTypeShape::Category category) const {
        if (category == ElementTypeShape::Category::Brick) {
            return coeffs[0][0][0] * 8
                + (coeffs[2][0][0] + coeffs[0][2][0] + coeffs[0][0][2]) * 8 / 3
                + (coeffs[2][2][0] + coeffs[0][2][2] + coeffs[2][0][2]) * 8 / 9
                + coeffs[2][2][2] * 8 / 27;
        } else if (category == ElementTypeShape::Category::Tetrahedron) {
            return coeffs[0][0][0] / 6
                + (coeffs[1][0][0] + coeffs[0][1][0] + coeffs[0][0][1]) / 24
                + (coeffs[1][1][0] + coeffs[0][1][1] + coeffs[1][0][1]) / 120
                + (coeffs[2][0][0] + coeffs[0][2][0] + coeffs[0][0][2]) / 60
                + coeffs[1][1][1] / 720
                + (coeffs[2][1][0] + coeffs[0][2][1] + coeffs[1][0][2]
                    + coeffs[0][1][2] + coeffs[1][2][0] + coeffs[2][0][1]) / 360
                + (coeffs[2][1][1] + coeffs[1][2][1] + coeffs[1][1][2]) / 2520
                + (coeffs[2][2][0] + coeffs[0][2][2] + coeffs[2][0][2]) / 1260
                + (coeffs[2][2][1] + coeffs[1][2][2] + coeffs[2][1][2]) / 10080
                + coeffs[2][2][2] / 45360;
        } else {
            assert(false);
        }
    }
};

void check_element_type_shape_integration(
    const ElementTypeShape &shape,
    const QuadraticUVWPolynomial &test_poly
) {
    double numeric_result = 0;
    for (const auto &ip : shape.volume_integration_points) {
        numeric_result += ip.weight * test_poly.evaluate(ip.uvw);
    }

    double symbolic_result = test_poly.integrate(shape.category);

    EXPECT_NEAR(symbolic_result, numeric_result, 1e-10);
}

void check_element_type_shape(const ElementTypeShape &shape) {
    for (int i = 0; i < 5; ++i) {
        double u, v, w;
        if (shape.category == ElementTypeShape::Category::Brick) {
            u = rand_float(-1, 1);
            v = rand_float(-1, 1);
            w = rand_float(-1, 1);
        } else if (shape.category == ElementTypeShape::Category::Tetrahedron) {
            u = rand_float(0, 1);
            v = rand_float(0, u);
            w = rand_float(0, v);
        } else {
            assert(false);
        }
        check_element_type_shape_functions(
            shape, ElementTypeShape::ShapePoint(u, v, w));
    }

    check_element_type_shape_integration(
        shape, QuadraticUVWPolynomial::random(0));
    check_element_type_shape_integration(
        shape, QuadraticUVWPolynomial::random(1));
    if (shape.order == 2) {
        check_element_type_shape_integration(
            shape, QuadraticUVWPolynomial::random(2));
    }
}

TEST(MeshTypeInfoTest, C3D8) {
    check_element_type_shape(element_type_shape(ElementType::C3D8));
}
TEST(MeshTypeInfoTest, C3D20) {
    check_element_type_shape(element_type_shape(ElementType::C3D20));
}
TEST(MeshTypeInfoTest, C3D20R) {
    check_element_type_shape(element_type_shape(ElementType::C3D20R));
}
TEST(MeshTypeInfoTest, C3D20RI) {
    check_element_type_shape(element_type_shape(ElementType::C3D20RI));
}
TEST(MeshTypeInfoTest, C3D4) {
    check_element_type_shape(element_type_shape(ElementType::C3D4));
}
TEST(MeshTypeInfoTest, C3D10) {
    check_element_type_shape(element_type_shape(ElementType::C3D10));
}

} /* namespace os2cx */
