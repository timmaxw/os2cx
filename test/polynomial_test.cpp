#include <gtest/gtest.h>

#include "mesh_index.hpp"

namespace os2cx {

inline Polynomial p(Polynomial::Variable v) { return Polynomial(v); }
inline Polynomial p(double v) { return Polynomial(v); }

Polynomial::Variable v1 { 1 };
Polynomial::Variable v2 { 2 };
Polynomial::Variable v3 { 3 };
Polynomial p1 = p(123) + p(v1);
Polynomial p2 = p(-123) + pow(p(v2), 2);
Polynomial p3 = p(v1) * p(v2) * p(v3);

TEST(PolynomialTest, Negation) {
    EXPECT_EQ(p(-123), -p(123));
}

TEST(PolynomialTest, Addition) {
    Polynomial answer = p(v1) + pow(p(v2), 2);
    EXPECT_EQ(answer, p1 + p2);
    Polynomial p1_copy = p1;
    p1_copy += p2;
    EXPECT_EQ(answer, p1_copy);
}

TEST(PolynomialTest, Subtraction) {
    Polynomial answer = p(246) + p(v1) - pow(p(v2), 2);
    EXPECT_EQ(answer, p1 - p2);
    Polynomial p1_copy = p1;
    p1_copy -= p2;
    EXPECT_EQ(answer, p1_copy);
}

TEST(PolynomialTest, Multiplication) {
    Polynomial answer =
        p(-15129) +
        p(-123) * p(v1) +
        p(123) * pow(p(v2), 2) +
        p(v1) * pow(p(v2), 2);
    EXPECT_EQ(answer, p1 * p2);
    Polynomial p1_copy = p1;
    p1_copy *= p2;
    EXPECT_EQ(answer, p1_copy);
}

TEST(PolynomialTest, Division) {
    Polynomial answer = p(61.5) + p(0.5) * p(v1);
    EXPECT_EQ(answer, p1 / 2);
    Polynomial p1_copy = p1;
    p1_copy /= 2;
    EXPECT_EQ(answer, p1_copy);
}

TEST(PolynomialTest, Power) {
    Polynomial answer = p(15129) + p(246) * p(v1) + p(v1) * p(v1);
    EXPECT_EQ(answer, pow(p1, 2));
}

TEST(PolynomialTest, EvaluatePartial) {
    Polynomial answer = p(124);
    EXPECT_EQ(answer, p1.evaluate_partial(v1, p(1)));

    Polynomial answer2 = p(-123) * p(v2) * p(v3) + pow(p(v2), 3) * p(v3);
    EXPECT_EQ(answer2, p3.evaluate_partial(v1, p2));
}

TEST(PolynomialTest, Differentiate) {
    Polynomial answer = p(2) * p(v2);
    EXPECT_EQ(answer, p2.differentiate(v2));
}

TEST(PolynomialTest, Antidifferentiate) {
    Polynomial answer = p(-123) * p(v2) + p(1.0 / 3) * pow(p(v2), 3);
    EXPECT_EQ(answer, p2.antidifferentiate(v2));
}

TEST(PolynomialTest, Integrate) {
    Polynomial result = (p(v1)*p(v1)).integrate(v1, p(0), p(1));
    EXPECT_EQ(p(1/3.0), result);
}

} /* namespace os2cx */
