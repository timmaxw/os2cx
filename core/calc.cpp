#include "calc.hpp"

#include <assert.h>

namespace os2cx {

std::ostream &operator<<(std::ostream &stream, Vector vector) {
    return stream << '['
        << vector.x << ' '
        << vector.y << ' '
        << vector.z << ']';
}

Vector triangle_normal(Point p1, Point p2, Point p3) {
    LengthVector v1 = p2 - p1;
    LengthVector v2 = p3 - p1;
    Vector c = v1.cross(v2);
    return c / c.magnitude();
}

std::ostream &operator<<(std::ostream &stream, Point point) {
    return stream << '('
        << point.x << ' '
        << point.y << ' '
        << point.z << ')';
}

Matrix Matrix::zero() {
    Matrix m;
    m.cols[0] = Vector(0, 0, 0);
    m.cols[1] = Vector(0, 0, 0);
    m.cols[2] = Vector(0, 0, 0);
    return m;
}

Matrix Matrix::identity() {
    Matrix m;
    m.cols[0] = Vector(1, 0, 0);
    m.cols[1] = Vector(0, 1, 0);
    m.cols[2] = Vector(0, 0, 1);
    return m;
}

Matrix Matrix::rotation(int axis, double radians) {
    Matrix m;
    if (axis == 0) {
        m.cols[0] = Vector(1, 0, 0);
        m.cols[1] = Vector(0, cos(radians), sin(radians));
        m.cols[2] = Vector(0, -sin(radians), cos(radians));
    } else if (axis == 1) {
        m.cols[0] = Vector(cos(radians), 0, -sin(radians));
        m.cols[1] = Vector(0, 1, 0);
        m.cols[2] = Vector(sin(radians), 0, cos(radians));
    } else if (axis == 2) {
        m.cols[0] = Vector(cos(radians), sin(radians), 0);
        m.cols[1] = Vector(-sin(radians), cos(radians), 0);
        m.cols[2] = Vector(0, 0, 1);
    } else {
        assert(false);
    }
    return m;
}

Matrix Matrix::scale(double xs, double ys, double zs) {
    Matrix m;
    m.cols[0] = Vector(xs, 0, 0);
    m.cols[1] = Vector(0, ys, 0);
    m.cols[2] = Vector(0, 0, zs);
    return m;
}

double Matrix::determinant() const {
    return cols[0].cross(cols[1]).dot(cols[2]);
}

Matrix Matrix::cofactor_matrix() const
{
    Matrix m;
    m.cols[0] = cols[1].cross(cols[2]);
    m.cols[1] = cols[2].cross(cols[0]);
    m.cols[2] = cols[0].cross(cols[1]);
    return m;
}

std::ostream &operator<<(std::ostream &stream, Box box) {
    return stream << "Box("
        << box.xl << ", " << box.yl << ", " << box.zl << ", "
        << box.xh << ", " << box.yh << ", " << box.zh << ")";
}

} /* namespace os2cx */
