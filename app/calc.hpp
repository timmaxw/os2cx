#ifndef OS2CX_CALC_HPP_
#define OS2CX_CALC_HPP_

#include <math.h>

#include <iostream>

namespace os2cx {

std::string unit_name(int m, int kg, int s);
std::string unit_abbreviation(int m, int kg, int s);

template<int m, int kg, int s>
class Scalar {
public:
    static const int m_exp = m;
    static const int kg_exp = kg;
    static const int s_exp = s;

    Scalar() { }
    explicit Scalar(double v) : val(v) { }

    Scalar operator+(Scalar other) const { return Scalar(val + other.val); }
    Scalar operator-(Scalar other) const { return Scalar(val - other.val); }
    Scalar operator-() const { return Scalar(-val); }
    Scalar &operator+=(Scalar other) { val += other.val; return *this; }
    Scalar &operator-=(Scalar other) { val -= other.val; return *this; }

    template<int m2, int kg2, int s2>
    Scalar<m + m2, kg + kg2, s + s2>
    operator*(Scalar<m2, kg2, s2> other) const {
        return Scalar<m + m2, kg + kg2, s + s2>(val * other.val);
    }
    template<int m2, int kg2, int s2>
    Scalar<m - m2, kg - kg2, s - s2>
    operator/(Scalar<m2, kg2, s2> other) const {
        return Scalar<m - m2, kg - kg2, s - s2>(val / other.val);
    }

    Scalar operator*(double other) const { return Scalar(val * other); }
    Scalar operator/(double other) const { return Scalar(val / other); }
    Scalar &operator*=(double other) { val *= other; return *this; }
    Scalar &operator/=(double other) { val /= other; return *this; }

    bool operator==(Scalar other) const { return val == other.val; }
    bool operator!=(Scalar other) const { return val != other.val; }
    bool operator>(Scalar other) const { return val > other.val; }
    bool operator<(Scalar other) const { return val < other.val; }
    bool operator>=(Scalar other) const { return val >= other.val; }
    bool operator<=(Scalar other) const { return val <= other.val; }

    double val;
};

template<int m, int kg, int s>
Scalar<m, kg, s> operator*(double a, Scalar<m, kg, s> b) {
    return b * a;
}

template<int m, int kg, int s>
Scalar<m, kg, s> operator/(double a, Scalar<m, kg, s> b) {
    return Scalar<0, 0, 0>(a) / b;
}

template<int m, int kg, int s>
std::ostream &operator<<(std::ostream &stream, Scalar<m, kg, s> scalar) {
    return stream << scalar.val << unit_abbreviation(m, kg, s);
}

template<int m, int kg, int s>
class Vector {
public:
    static const int m_exp = m;
    static const int kg_exp = kg;
    static const int s_exp = s;

    typedef Scalar<m, kg, s> S;

    static Vector zero() {
        return Vector(S(0), S(0), S(0));
    }

    static Vector raw(double x, double y, double z) {
        return Vector(S(x), S(y), S(z));
    }

    Vector() { }
    Vector(S x_, S y_, S z_) : x(x_), y(y_), z(z_) { }

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
    
    template<int m2, int kg2, int s2>
    Vector<m + m2, kg + kg2, s + s2>
    operator*(Scalar<m2, kg2, s2> sc) const {
        return Vector<m + m2, kg + kg2, s + s2>(x * sc, y * sc, z * sc);
    }
    template<int m2, int kg2, int s2>
    Vector<m - m2, kg - kg2, s - s2>
    operator/(Scalar<m2, kg2, s2> sc) const {
        return Vector<m - m2, kg - kg2, s - s2>(x / sc, y / sc, z / sc);
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

    S magnitude() const {
        return S(sqrt(x.val * x.val + y.val * y.val + z.val * z.val));
    }

    template<int m2, int kg2, int s2>
    Scalar<m + m2, kg + kg2, s + s2> dot(Vector<m2, kg2, s2> other) const {
        return Scalar<m + m2, kg + kg2, s + s2>(
            x * other.x + y * other.y + z * other.z);
    }
    template<int m2, int kg2, int s2>
    Vector<m + m2, kg + kg2, s + s2> cross(Vector<m2, kg2, s2> other) const {
        return Vector<m + m2, kg + kg2, s + s2>(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x);
    }

    S x, y, z;
};

template<int m, int kg, int s>
Vector<m, kg, s> operator*(double a, Vector<m, kg, s> b) {
    return b * a;
}

template<int m, int kg, int s, int m2, int kg2, int s2>
Vector<m + m2, kg + kg2, s + s2>
operator*(Scalar<m, kg, s> a, Vector<m2, kg2, s2> b) {
    return b * a;
}

template<int m, int kg, int s>
std::ostream &operator<<(std::ostream &stream, Vector<m, kg, s> vector) {
    return stream << '['
        << vector.x.val << ' '
        << vector.y.val << ' '
        << vector.z.val << ']'
        << unit_abbreviation(m, kg, s);
}

typedef Scalar<0, 0, 0> PureScalar;
typedef Scalar<1, 0, 0> Length;
typedef Scalar<2, 0, 0> Area;
typedef Scalar<3, 0, 0> Volume;
typedef Scalar<1, 1, -2> Force;
typedef Scalar<-1, 1, -2> Pressure;
typedef Scalar<-2, 1, -2> ForceDensity;

typedef Vector<0, 0, 0> PureVector;
typedef Vector<1, 0, 0> LengthVector;
typedef Vector<1, 1, -2> ForceVector;
typedef Vector<-2, 1, -2> ForceDensityVector;

class Point {
public:
    static Point raw(double x, double y, double z) {
        return Point(LengthVector::raw(x, y, z));
    }

    Point() { }
    explicit Point(LengthVector vector_) : vector(vector_) { }

    Point operator+(LengthVector other) const {
        return Point(vector + other);
    }
    Point operator-(LengthVector other) const {
        return Point(vector - other);
    }
    LengthVector operator-(Point other) const {
        return vector - other.vector;
    }
    Point &operator+=(LengthVector other) {
        vector += other;
        return *this;
    }
    Point &operator-=(LengthVector other) {
        vector -= other;
        return *this;
    }

    bool operator==(Point other) const {
        return vector == other.vector;
    }
    bool operator!=(Point other) const {
        return vector != other.vector;
    }

    LengthVector vector;
};

PureVector triangle_normal(Point p1, Point p2, Point p3);

inline std::ostream &operator<<(std::ostream &stream, Point point) {
    return stream << '('
        << point.vector.x.val << ' '
        << point.vector.y.val << ' '
        << point.vector.z.val << ')';
}

} /* namespace os2cx */

#endif
