//
// Created by bruno on 14/05/22.
//

#ifndef MANDELBROT_VEC2_H
#define MANDELBROT_VEC2_H

#include <cmath>
#include <iostream>

using std::sqrt;

class vec2 {
public:
    vec2() : e{0,0} {}
    vec2(double e0, double e1) : e{e0, e1} {}

    double x() const { return e[0]; }
    double y() const { return e[1]; }

    vec2 operator-() const { return vec2(-e[0], -e[1]); }
    double operator[](int i) const { return e[i]; }
    double& operator[](int i) { return e[i]; }

    vec2& operator+=(const vec2 &v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        return *this;
    }

    vec2& operator*=(const double t) {
        e[0] *= t;
        e[1] *= t;
        return *this;
    }

    vec2& operator/=(const double t) {
        return *this *= 1/t;
    }

    vec2 copy() const {
        return vec2(this->x(), this->y());
    }

    double length() const {
        return sqrt(length_squared());
    }

    double length_squared() const {
        return e[0]*e[0] + e[1]*e[1];
    }

public:
    double e[2];
};

inline std::ostream& operator<<(std::ostream &out, const vec2 &v) {
    return out << v.e[0] << ' ' << v.e[1];
}

inline vec2 operator+(const vec2 &u, const vec2 &v) {
    return vec2(u.e[0] + v.e[0], u.e[1] + v.e[1]);
}

inline vec2 operator-(const vec2 &u, const vec2 &v) {
    return vec2(u.e[0] - v.e[0], u.e[1] - v.e[1]);
}

inline vec2 operator*(const vec2 &u, const vec2 &v) {
    return vec2(u.e[0] * v.e[0], u.e[1] * v.e[1]);
}

inline vec2 operator*(double t, const vec2 &v) {
    return vec2(t*v.e[0], t*v.e[1]);
}

inline vec2 operator*(const vec2 &v, double t) {
    return t * v;
}

inline vec2 operator/(vec2 v, double t) {
    return (1/t) * v;
}

#endif //MANDELBROT_VEC2_H
