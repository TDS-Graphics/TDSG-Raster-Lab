#pragma once

#include "math/mat.h"

inline Mat4f identity4() {
    return Mat4f{{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    }};
}

inline Mat4f translate(const Vec3f& t) {
    return Mat4f{{
        {1.0f, 0.0f, 0.0f, t.x},
        {0.0f, 1.0f, 0.0f, t.y},
        {0.0f, 0.0f, 1.0f, t.z},
        {0.0f, 0.0f, 0.0f, 1.0f},
    }};
}

inline Mat4f scale(const Vec3f& s) {
    return Mat4f{{
        {s.x,  0.0f, 0.0f, 0.0f},
        {0.0f, s.y,  0.0f, 0.0f},
        {0.0f, 0.0f, s.z,  0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    }};
}

inline Mat4f rotate_x(float radians) {
    const float c = std::cos(radians), s = std::sin(radians);
    return Mat4f{{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, c,    -s,   0.0f},
        {0.0f, s,     c,   0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    }};
}

inline Mat4f rotate_y(float radians) {
    const float c = std::cos(radians), s = std::sin(radians);
    return Mat4f{{
        { c,   0.0f, s,    0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {-s,   0.0f, c,    0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    }};
}

inline Mat4f rotate_z(float radians) {
    const float c = std::cos(radians), s = std::sin(radians);
    return Mat4f{{
        {c,    -s,   0.0f, 0.0f},
        {s,     c,   0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    }};
}

// from Rodrigues' rotation formula
inline Mat4f rotate(const Vec3f& axis, float radians) {
    const Vec3f a = normalize(axis);
    const float c = std::cos(radians), s = std::sin(radians), t = 1.0f - c;
    return Mat4f{{
        {t * a.x * a.x + c,       t * a.x * a.y - s * a.z, t * a.x * a.z + s * a.y, 0.0f},
        {t * a.x * a.y + s * a.z, t * a.y * a.y + c,       t * a.y * a.z - s * a.x, 0.0f},
        {t * a.x * a.z - s * a.y, t * a.y * a.z + s * a.x, t * a.z * a.z + c,       0.0f},
        {0.0f,                    0.0f,                    0.0f,                    1.0f},
    }};
}

inline Mat4f perspective(float fov_y, float aspect, float near_plane, float far_plane) {
    const float f = 1.0f / std::tan(fov_y * 0.5f);
    const float d = near_plane - far_plane;
    return Mat4f{{
        {f / aspect, 0.0f, 0.0f,                        0.0f},
        {0.0f,       f,    0.0f,                        0.0f},
        {0.0f,       0.0f, (far_plane + near_plane) / d, (2.0f * far_plane * near_plane) / d},
        {0.0f,       0.0f, -1.0f,                       0.0f},
    }};
}

inline Mat4f orthographic(float l, float r, float b, float t, float n, float f) {
    return Mat4f{{
        {2.0f / (r - l), 0.0f,           0.0f,           -(r + l) / (r - l)},
        {0.0f,           2.0f / (t - b), 0.0f,           -(t + b) / (t - b)},
        {0.0f,           0.0f,           -2.0f / (f - n), -(f + n) / (f - n)},
        {0.0f,           0.0f,           0.0f,           1.0f},
    }};
}

inline Mat4f look_at(const Vec3f& eye, const Vec3f& center, const Vec3f& up) {
    const Vec3f f = normalize(center - eye);
    const Vec3f s = normalize(cross(f, up));
    const Vec3f u = cross(s, f);
    return Mat4f{{
        {s.x,   s.y,   s.z,   -dot(s, eye)},
        {u.x,   u.y,   u.z,   -dot(u, eye)},
        {-f.x,  -f.y,  -f.z,  dot(f, eye)},
        {0.0f,  0.0f,  0.0f,  1.0f},
    }};
}
