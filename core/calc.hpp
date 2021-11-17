#ifndef OS2CX_CALC_HPP_
#define OS2CX_CALC_HPP_

#include <assert.h>
#include <math.h>

#include <complex>
#include <iostream>

namespace os2cx {

typedef double Length;
typedef double Volume;

enum class Dimension {X, Y, Z};

std::ostream &operator<<(std::ostream &stream, Dimension dimension);

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
    double at(Dimension d) const {
        switch (d) {
        case Dimension::X: return x;
        case Dimension::Y: return y;
        case Dimension::Z: return z;
        default: assert(false);
        }
    }
    void set_at(Dimension d, double value) {
        switch (d) {
        case Dimension::X: x = value; break;
        case Dimension::Y: y = value; break;
        case Dimension::Z: z = value; break;
        default: assert(false);
        }
    }

    double x, y, z;
};

inline Vector operator*(double a, Vector b) {
    return b * a;
}

std::ostream &operator<<(std::ostream &stream, Vector vector);

typedef Vector LengthVector;

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
    double at(Dimension d) const {
        switch (d) {
        case Dimension::X: return x;
        case Dimension::Y: return y;
        case Dimension::Z: return z;
        default: assert(false);
        }
    }
    void set_at(Dimension d, double value) {
        switch (d) {
        case Dimension::X: x = value; break;
        case Dimension::Y: y = value; break;
        case Dimension::Z: z = value; break;
        default: assert(false);
        }
    }

    double x, y, z;
};

inline Point operator+(LengthVector a, Point b) {
    return b + a;
}

Vector triangle_normal(Point p1, Point p2, Point p3);

std::ostream &operator<<(std::ostream &stream, Point point);

class ComplexVector {
public:
    static ComplexVector zero() {
        return ComplexVector(0, 0, 0);
    }

    ComplexVector() { }
    ComplexVector(
        std::complex<double> x_,
        std::complex<double> y_,
        std::complex<double> z_
    ) : x(x_), y(y_), z(z_) { }
    ComplexVector(Vector r, Vector i)
        : x(r.x, i.x), y(r.y, i.y), z(r.z, i.z) { }

    ComplexVector operator+(ComplexVector other) const {
        return ComplexVector(x + other.x, y + other.y, z + other.z);
    }
    ComplexVector operator-(ComplexVector other) const {
        return ComplexVector(x - other.x, y - other.y, z - other.z);
    }
    ComplexVector operator-() const {
        return ComplexVector(-x, -y, -z);
    }
    ComplexVector &operator+=(ComplexVector other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    ComplexVector &operator-=(ComplexVector other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
    ComplexVector operator*(std::complex<double> other) const {
        return ComplexVector(x * other, y * other, z * other);
    }
    ComplexVector operator/(std::complex<double> other) const {
        return ComplexVector(x / other, y / other, z / other);
    }
    ComplexVector &operator*=(std::complex<double> other) {
        x *= other;
        y *= other;
        z *= other;
        return *this;
    }
    ComplexVector &operator/=(std::complex<double> other) {
        x /= other;
        y /= other;
        z /= other;
        return *this;
    }
    bool operator==(ComplexVector other) const {
        return x == other.x && y == other.y && z == other.z;
    }
    bool operator!=(ComplexVector other) const {
        return !(*this == other);
    }
    double magnitude() const {
        return sqrt(std::norm(x) + std::norm(y) + std::norm(z));
    }
    Vector real() const {
        return Vector(x.real(), y.real(), z.real());
    }
    Vector imag() const {
        return Vector(x.imag(), y.imag(), z.imag());
    }
    std::complex<double> dot(ComplexVector other) const {
        return x * std::conj(other.x)
             + y * std::conj(other.y)
             + z * std::conj(other.z);
    }
    std::complex<double> at(Dimension d) const {
        switch (d) {
        case Dimension::X: return x;
        case Dimension::Y: return y;
        case Dimension::Z: return z;
        default: assert(false);
        }
    }
    void set_at(Dimension d, std::complex<double> value) {
        switch (d) {
        case Dimension::X: x = value; break;
        case Dimension::Y: y = value; break;
        case Dimension::Z: z = value; break;
        default: assert(false);
        }
    }

    std::complex<double> x, y, z;
};

inline ComplexVector operator*(std::complex<double> a, ComplexVector b) {
    return b * a;
}

std::ostream &operator<<(std::ostream &stream, ComplexVector vector);

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

double von_mises_stress(const Matrix &m);

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
