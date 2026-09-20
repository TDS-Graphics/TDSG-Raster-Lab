#pragma once

#include <cmath>
#include <cstddef>

template <int N, typename T = float>
struct Vec {
    T data[N]{};

    T&       operator[](int i)       { return data[i]; }
    const T& operator[](int i) const { return data[i]; }

    static int size() { return N; }
};

template <typename T>
struct Vec<2, T> {
    T x{}, y{};

    Vec() {}
    Vec(T x_, T y_) : x(x_), y(y_) {}
    explicit Vec(T s) : x(s), y(s) {}

    T&       operator[](int i)       { return i == 0 ? x : y; }
    const T& operator[](int i) const { return i == 0 ? x : y; }

    static int size() { return 2; }
};

template <typename T>
struct Vec<3, T> {
    T x{}, y{}, z{};

    Vec() {}
    Vec(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}
    explicit Vec(T s) : x(s), y(s), z(s) {}

    T&       operator[](int i)       { return i == 0 ? x : (i == 1 ? y : z); }
    const T& operator[](int i) const { return i == 0 ? x : (i == 1 ? y : z); }

    static int size() { return 3; }
};

template <typename T>
struct Vec<4, T> {
    T x{}, y{}, z{}, w{};

    Vec() {}
    Vec(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}
    Vec(const Vec<3, T>& v, T w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
    explicit Vec(T s) : x(s), y(s), z(s), w(s) {}

    T&       operator[](int i)       { return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w)); }
    const T& operator[](int i) const { return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w)); }

    Vec<3, T> xyz() const { return Vec<3, T>(x, y, z); }
    Vec<2, T> xy()  const { return Vec<2, T>(x, y); }

    static int size() { return 4; }
};

using Vec2f = Vec<2, float>;
using Vec3f = Vec<3, float>;
using Vec4f = Vec<4, float>;
using Vec2d = Vec<2, double>;
using Vec3d = Vec<3, double>;
using Vec4d = Vec<4, double>;
using Vec2i = Vec<2, int>;
using Vec3i = Vec<3, int>;

template <int N, typename T>
Vec<N, T> operator+(const Vec<N, T>& a, const Vec<N, T>& b) {
    Vec<N, T> r{};
    for (int i = 0; i < N; ++i) r[i] = a[i] + b[i];
    return r;
}

template <int N, typename T>
Vec<N, T> operator-(const Vec<N, T>& a, const Vec<N, T>& b) {
    Vec<N, T> r{};
    for (int i = 0; i < N; ++i) r[i] = a[i] - b[i];
    return r;
}

template <int N, typename T>
Vec<N, T> operator-(const Vec<N, T>& a) {
    Vec<N, T> r{};
    for (int i = 0; i < N; ++i) r[i] = -a[i];
    return r;
}

template <int N, typename T>
Vec<N, T> operator*(const Vec<N, T>& a, T s) {
    Vec<N, T> r{};
    for (int i = 0; i < N; ++i) r[i] = a[i] * s;
    return r;
}

template <int N, typename T>
Vec<N, T> operator*(T s, const Vec<N, T>& a) {
    return a * s;
}

template <int N, typename T>
Vec<N, T> operator/(const Vec<N, T>& a, T s) {
    Vec<N, T> r{};
    for (int i = 0; i < N; ++i) r[i] = a[i] / s;
    return r;
}

template <int N, typename T>
Vec<N, T>& operator+=(Vec<N, T>& a, const Vec<N, T>& b) {
    for (int i = 0; i < N; ++i) a[i] += b[i];
    return a;
}

template <int N, typename T>
Vec<N, T>& operator-=(Vec<N, T>& a, const Vec<N, T>& b) {
    for (int i = 0; i < N; ++i) a[i] -= b[i];
    return a;
}

template <int N, typename T>
Vec<N, T>& operator*=(Vec<N, T>& a, T s) {
    for (int i = 0; i < N; ++i) a[i] *= s;
    return a;
}

template <int N, typename T>
Vec<N, T> cwise_mul(const Vec<N, T>& a, const Vec<N, T>& b) {
    Vec<N, T> r{};
    for (int i = 0; i < N; ++i) r[i] = a[i] * b[i];
    return r;
}

template <int N, typename T>
T dot(const Vec<N, T>& a, const Vec<N, T>& b) {
    T r = T(0);
    for (int i = 0; i < N; ++i) r += a[i] * b[i];
    return r;
}

template <typename T>
Vec<3, T> cross(const Vec<3, T>& a, const Vec<3, T>& b) {
    return Vec<3, T>(a.y * b.z - a.z * b.y,
                     a.z * b.x - a.x * b.z,
                     a.x * b.y - a.y * b.x);
}

template <int N, typename T>
inline T length(const Vec<N, T>& v) {
    return std::sqrt(dot(v, v));
}

template <int N, typename T>
inline Vec<N, T> normalize(const Vec<N, T>& v) {
    const T len = length(v);
    return len > T(0) ? v / len : v;
}
