#pragma once

#include <algorithm>
#include <cmath>

#include "math/vec.h"

template <int R, int C, typename T = float>
struct Mat {
    Vec<C, T> rows[R]{};

    Vec<C, T>&       operator[](int i)       { return rows[i]; }
    const Vec<C, T>& operator[](int i) const { return rows[i]; }

    static const int num_rows = R;
    static const int num_cols = C;

    static Mat<R, C, T> zero() { return Mat<R, C, T>{}; }

    static Mat<R, C, T> identity() {
        Mat<R, C, T> m{};
        for (int i = 0; i < R && i < C; ++i) m[i][i] = T(1);
        return m;
    }
};

using Mat3f = Mat<3, 3, float>;
using Mat4f = Mat<4, 4, float>;
using Mat3d = Mat<3, 3, double>;
using Mat4d = Mat<4, 4, double>;

template <int R, int C, typename T>
Vec<R, T> operator*(const Mat<R, C, T>& m, const Vec<C, T>& v) {
    Vec<R, T> r{};
    for (int i = 0; i < R; ++i) r[i] = dot(m[i], v);
    return r;
}

template <int R, int C, typename T>
Vec<C, T> operator*(const Vec<R, T>& v, const Mat<R, C, T>& m) {
    Vec<C, T> r{};
    for (int i = 0; i < R; ++i)
        for (int j = 0; j < C; ++j) r[j] += v[i] * m[i][j];
    return r;
}

template <int R, int C1, int C2, typename T>
Mat<R, C2, T> operator*(const Mat<R, C1, T>& a, const Mat<C1, C2, T>& b) {
    Mat<R, C2, T> r{};
    for (int i = 0; i < R; ++i)
        for (int k = 0; k < C1; ++k)
            for (int j = 0; j < C2; ++j) r[i][j] += a[i][k] * b[k][j];
    return r;
}

template <int R, int C, typename T>
Mat<R, C, T> operator*(const Mat<R, C, T>& m, T s) {
    Mat<R, C, T> r{};
    for (int i = 0; i < R; ++i) r[i] = m[i] * s;
    return r;
}

template <int R, int C, typename T>
Mat<R, C, T> operator*(T s, const Mat<R, C, T>& m) {
    return m * s;
}

template <int R, int C, typename T>
Mat<R, C, T> operator/(const Mat<R, C, T>& m, T s) {
    Mat<R, C, T> r{};
    for (int i = 0; i < R; ++i) r[i] = m[i] / s;
    return r;
}

template <int R, int C, typename T>
Mat<R, C, T> operator+(const Mat<R, C, T>& a, const Mat<R, C, T>& b) {
    Mat<R, C, T> r{};
    for (int i = 0; i < R; ++i) r[i] = a[i] + b[i];
    return r;
}

template <int R, int C, typename T>
Mat<R, C, T> operator-(const Mat<R, C, T>& a, const Mat<R, C, T>& b) {
    Mat<R, C, T> r{};
    for (int i = 0; i < R; ++i) r[i] = a[i] - b[i];
    return r;
}

template <int R, int C, typename T>
Mat<C, R, T> transpose(const Mat<R, C, T>& m) {
    Mat<C, R, T> out{};
    for (int i = 0; i < R; ++i)
        for (int j = 0; j < C; ++j) out[j][i] = m[i][j];
    return out;
}

template <typename T>
T determinant(const Mat<3, 3, T>& m) {
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
         - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
         + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

template <typename T>
T determinant(const Mat<4, 4, T>& m) {
    T det = T(0);
    for (int col = 0; col < 4; ++col) {
        Mat<3, 3, T> sub;
        for (int i = 0; i < 3; ++i) {
            int c = 0;
            for (int j = 0; j < 4; ++j) {
                if (j == col) continue;
                sub[i][c] = m[i + 1][j];
                ++c;
            }
        }
        det += (col % 2 == 0 ? T(1) : T(-1)) * m[0][col] * determinant(sub);
    }
    return det;
}

template <int N, typename T>
Mat<N, N, T> inverse(const Mat<N, N, T>& m) {
    T a[N][2 * N];
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) a[i][j] = m[i][j];
        for (int j = 0; j < N; ++j) a[i][N + j] = (i == j) ? T(1) : T(0);
    }

    for (int col = 0; col < N; ++col) {
        int best = col;
        for (int row = col + 1; row < N; ++row)
            if (std::abs(a[row][col]) > std::abs(a[best][col])) best = row;
        for (int j = 0; j < 2 * N; ++j) std::swap(a[col][j], a[best][j]);

        const T pivot = a[col][col];
        for (int j = 0; j < 2 * N; ++j) a[col][j] /= pivot;

        for (int row = 0; row < N; ++row) {
            if (row == col) continue;
            const T factor = a[row][col];
            for (int j = 0; j < 2 * N; ++j) a[row][j] -= factor * a[col][j];
        }
    }

    Mat<N, N, T> out;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) out[i][j] = a[i][N + j];
    return out;
}

inline Mat3f upper_left3(const Mat4f& m) {
    Mat3f out{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) out[i][j] = m[i][j];
    return out;
}
