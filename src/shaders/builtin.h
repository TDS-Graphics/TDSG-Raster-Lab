#pragma once

#include <algorithm>
#include <cmath>

#include "core/shader.h"

class FlatShader : public Shader {
public:
    Color fragment(const Fragment& f, const Uniforms&) const override {
        return Color(f.color.x, f.color.y, f.color.z, f.color.w);
    }
};

class NormalShader : public Shader {
public:
    Color fragment(const Fragment& f, const Uniforms&) const override {
        const Vec3f n = normalize(f.normal);
        return Color(n.x * 0.5f + 0.5f, n.y * 0.5f + 0.5f, n.z * 0.5f + 0.5f);
    }
};

class PhongShader : public Shader {
public:
    Color fragment(const Fragment& f, const Uniforms& u) const override {
        const Material& m = u.material;
        const Vec3f n = normalize(f.normal);
        const Vec3f p = f.world_pos;
        const Vec3f view_dir = normalize(u.camera_position - p);

        Vec3f result = cwise_mul(m.ambient, u.ambient_light);

        for (const Light& light : u.lights) {
            const Vec3f to_light = light.position - p;
            const float distance_sq = std::max(dot(to_light, to_light), 1e-8f);
            const Vec3f light_dir = to_light / std::sqrt(distance_sq);

            const float n_dot_l = std::max(0.0f, dot(n, light_dir));
            const float n_dot_h = std::max(0.0f, dot(n, normalize(light_dir + view_dir)));

            const Vec3f radiance = light.intensity / distance_sq;
            result += cwise_mul(cwise_mul(m.diffuse, base_color(f, u)), radiance) * n_dot_l;
            result += cwise_mul(m.specular, radiance) * std::pow(n_dot_h, m.shininess);
        }
        return Color(result);
    }
};

class TextureShader : public Shader {
public:
    Color fragment(const Fragment& f, const Uniforms& u) const override {
        if (!u.material.texture || !u.material.texture->valid()) return Color::magenta();
        return u.material.texture->sample(f.uv);
    }
};

class DepthShader : public Shader {
public:
    Color fragment(const Fragment& f, const Uniforms&) const override {
        const float v = 1.0f - f.depth;
        return Color(v, v, v);
    }
};
