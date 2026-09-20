#pragma once

#include <cstdint>
#include <limits>
#include <vector>

#include "math/vec.h"

struct Vertex {
    Vec3f position;
    Vec3f normal{0.0f, 0.0f, 1.0f};
    Vec2f uv;
    Vec4f color{1.0f, 1.0f, 1.0f, 1.0f};
};

struct Fragment {
    Vec3f world_pos;
    Vec3f normal{0.0f, 0.0f, 1.0f};
    Vec2f uv;
    Vec4f color{1.0f, 1.0f, 1.0f, 1.0f};

    float depth = 0.0f;
};

inline Fragment operator+(const Fragment& a, const Fragment& b) {
    Fragment r;
    r.world_pos = a.world_pos + b.world_pos;
    r.normal    = a.normal + b.normal;
    r.uv        = a.uv + b.uv;
    r.color     = a.color + b.color;
    return r;
}

inline Fragment operator*(const Fragment& a, float s) {
    Fragment r;
    r.world_pos = a.world_pos * s;
    r.normal    = a.normal * s;
    r.uv        = a.uv * s;
    r.color     = a.color * s;
    return r;
}

inline Fragment operator*(float s, const Fragment& a) { return a * s; }

struct VSOutput {
    Vec4f clip_pos;
    Fragment vary;
};

struct Bounds3f {
    Vec3f min{std::numeric_limits<float>::max(),
              std::numeric_limits<float>::max(),
              std::numeric_limits<float>::max()};
    Vec3f max{-std::numeric_limits<float>::max(),
              -std::numeric_limits<float>::max(),
              -std::numeric_limits<float>::max()};

    bool empty() const { return min.x > max.x; }

    void expand(const Vec3f& p) {
        min.x = p.x < min.x ? p.x : min.x;
        min.y = p.y < min.y ? p.y : min.y;
        min.z = p.z < min.z ? p.z : min.z;
        max.x = p.x > max.x ? p.x : max.x;
        max.y = p.y > max.y ? p.y : max.y;
        max.z = p.z > max.z ? p.z : max.z;
    }

    Vec3f center() const { return (min + max) * 0.5f; }
    Vec3f extent() const { return max - min; }
    float radius() const { return length(extent()) * 0.5f; }
};
