#pragma once

#include <cstdint>

#include "core/geometry.h"
#include "core/mesh.h"
#include "core/shader.h"
#include "image/framebuffer.h"
#include "math/mat.h"

enum class CullMode { None, Back, Front };

class Rasterizer {
public:
    Rasterizer(Framebuffer& target) : target_(target) {}

    void set_shader(const Shader* shader) { shader_ = shader; }
    const Shader* shader() const { return shader_; }

    void set_uniforms(const Uniforms& uniforms) { uniforms_ = uniforms; }
    Uniforms& uniforms() { return uniforms_; }
    const Uniforms& uniforms() const { return uniforms_; }

    void draw(const Mesh& mesh);

    CullMode cull_mode = CullMode::Back;
    bool depth_test = true;
    bool depth_write = true;
    bool clip_frustum = true;
    int viewport_x = 0;
    int viewport_y = 0;
    int viewport_width = -1;
    int viewport_height = -1;

    std::size_t drawn_triangles = 0;
    std::size_t clipped_triangles = 0;
    std::size_t shaded_fragments = 0;

private:
    struct ScreenVertex {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        float inv_w = 1.0f;
        Fragment vary;
    };

    void emit_triangle(const VSOutput& a, const VSOutput& b, const VSOutput& c);
    void rasterize(const VSOutput& a, const VSOutput& b, const VSOutput& c);

    Framebuffer& target_;
    const Shader* shader_ = nullptr;
    Uniforms uniforms_{};
};
