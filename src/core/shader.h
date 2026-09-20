#pragma once

#include <vector>

#include "core/geometry.h"
#include "core/light.h"
#include "math/mat.h"
#include "math/transform.h"

struct Uniforms {
    Mat4f model = Mat4f::identity();
    Mat4f view = Mat4f::identity();
    Mat4f projection = Mat4f::identity();

    Vec3f camera_position{0.0f, 0.0f, 0.0f};
    std::vector<Light> lights;
    Vec3f ambient_light{0.05f, 0.05f, 0.05f};
    Material material;

    Mat4f mvp() const { return projection * view * model; }

    Mat3f normal_matrix() const { return transpose(inverse(upper_left3(model))); }
};

class Shader {
public:
    virtual ~Shader() {}

    virtual VSOutput vertex(const Vertex& v, const Uniforms& u) const {
        VSOutput out;
        out.clip_pos = u.mvp() * Vec4f(v.position, 1.0f);
        out.vary.world_pos = (u.model * Vec4f(v.position, 1.0f)).xyz();
        out.vary.normal = u.normal_matrix() * v.normal;
        out.vary.uv = v.uv;
        out.vary.color = v.color;
        return out;
    }

    virtual Color fragment(const Fragment& f, const Uniforms& u) const = 0;

    static Vec3f base_color(const Fragment& f, const Uniforms& u) {
        if (u.material.texture && u.material.texture->valid()) return u.material.texture->sample(f.uv).rgb();
        return Vec3f(f.color.x, f.color.y, f.color.z);
    }
};
