#ifndef OS2CX_CALC_HPP_
#define OS2CX_CALC_HPP_

#include <math.h>

#include <iostream>

namespace os2cx {

typedef double Length;
typedef double Volume;

class Vector {
public:
    static Vector zero() {
        return Vector(0, 0, 0);
    }

    Vector() { }
    Vector(double x_, double y_, double z_) : x(x_), y(y_), z(z_) { }

    Vector operator+(Vector other) const {
        return Vector(x + other.x, y + other.y, z + other.z);
    }
    Vector operator-(Vector other) const {
        return Vector(x - other.x, y - other.y, z - other.z);
    }
    Vector operator-() const {
        return Vector(-x, -y, -z);
    }
    Vector &operator+=(Vector other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    Vector &operator-=(Vector other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
    Vector operator*(double other) const {
        return Vector(x * other, y * other, z * other);
    }
    Vector operator/(double other) const {
        return Vector(x / other, y / other, z / other);
    }
    Vector &operator*=(double other) {
        x *= other;
        y *= other;
        z *= other;
        return *this;
    }
    Vector &operator/=(double other) {
        x /= other;
        y /= other;
        z /= other;
        return *this;
    }
    bool operator==(Vector other) const {
        return x == other.x && y == other.y && z == other.z;
    }
    bool operator!=(Vector other) const {
        return !(*this == other);
    }
    double magnitude() const {
        return sqrt(x * x + y * y + z * z);
    }
    double dot(Vector other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    Vector cross(Vector other) const {
        return Vector(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x);
    }

    double x, y, z;
};

inline Vector operator*(double a, Vector b) {
    return b * a;
}

typedef Vector LengthVector;

std::ostream &operator<<(std::ostream &stream, Vector vector);

class Point {
public:
    static Point origin() {
        return Point(0, 0, 0);
    }

    Point() { }
    Point (double x_, double y_, double z_) : x(x_), y(y_), z(z_) { }

    Point operator+(LengthVector other) const {
        return Point(x + other.x, y + other.y, z + other.z);
    }
    Point operator-(LengthVector other) const {
        return Point(x - other.x, y - other.y, z - other.z);
    }
    LengthVector operator-(Point other) const {
        return Vector(x - other.x, y - other.y, z - other.z);
    }
    Point &operator+=(LengthVector other) {
        *this = (*this + other);
        return *this;
    }
    Point &operator-=(LengthVector other) {
        *this = (*this - other);
        return *this;
    }
    bool operator==(Point other) const {
        return x == other.x && y == other.y && z == other.z;
    }
    bool operator!=(Point other) const {
        return x != other.x || y != other.y || z != other.z;
    }

    double x, y, z;
};

inline Point operator+(LengthVector a, Point b) {
    return b + a;
}

Vector triangle_normal(Point p1, Point p2, Point p3);

std::ostream &operator<<(std::ostream &stream, Point point);

class Matrix {
public:
    static Matrix zero();
    static Matrix identity();
    static Matrix rotation(int axis, double radians);
    static Matrix scale(double xs, double ys, double zs);

    Vector apply(Vector v) const {
        return v.x * cols[0] + v.y * cols[1] + v.z * cols[2];
    }

    double determinant() const;
    Matrix cofactor_matrix() const;

    Vector cols[3];
};

class AffineTransform {
public:
    AffineTransform() { }
    explicit AffineTransform(const Matrix &m) :
        matrix(m), vector(LengthVector::zero()) { }
    AffineTransform(const Matrix &m, const LengthVector &v) :
        matrix(m), vector(v) { }

    Point apply(Point v) const {
        return Point::origin() + matrix.apply(v - Point::origin()) + vector;
    }

    Matrix matrix;
    LengthVector vector;
};

class Box {
public:
    Box() { }
    Box(double _xl, double _yl, double _zl,
        double _xh, double _yh, double _zh) :
        xl(_xl), yl(_yl), zl(_zl), xh(_xh), yh(_yh), zh(_zh) { }
    bool operator==(const Box &o) const {
        return xl == o.xl && yl == o.yl && zl == o.zl
            && xh == o.xh && yh == o.yh && zh == o.zh;
    }
    bool operator!=(const Box &o) const {
        return !(*this == o);
    }
    bool contains(Point p) const {
        return (p.x >= xl && p.y >= yl && p.z >= zl &&
            p.x <= xh && p.y <= yh && p.z <= zh);
    }

    double xl, yl, zl, xh, yh, zh;
};

std::ostream &operator<<(std::ostream &stream, Box box);

} /* namespace os2cx */

#endif
