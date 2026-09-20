#pragma once

#include <cmath>
#include <numbers>

#include "core/mesh.h"

inline Mesh make_sphere(float radius = 1.0f, int segments = 48, int rings = 32) {
    std::vector<Vertex> verts;
    std::vector<std::uint32_t> indices;
    verts.reserve(segments + 1 * rings + 1);

    for (int r = 0; r <= rings; ++r) {
        const float v = static_cast<float>(r) / static_cast<float>(rings);
        const float phi = v * std::numbers::pi_v<float>;
        for (int s = 0; s <= segments; ++s) {
            const float u = static_cast<float>(s) / static_cast<float>(segments);
            const float theta = u * 2.0f * std::numbers::pi_v<float>;
            const Vec3f n(std::sin(phi) * std::cos(theta),
                          std::cos(phi),
                          std::sin(phi) * std::sin(theta));
            Vertex vert;
            vert.position = n * radius;
            vert.normal = n;
            vert.uv = Vec2f(u, 1.0f - v);
            vert.color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
            verts.push_back(vert);
        }
    }

    const std::uint32_t stride = static_cast<std::uint32_t>(segments + 1);
    for (std::uint32_t r = 0; r < static_cast<std::uint32_t>(rings); ++r) {
        for (std::uint32_t s = 0; s < static_cast<std::uint32_t>(segments); ++s) {
            const std::uint32_t a = r * stride + s;
            const std::uint32_t b = a + stride;
            indices.insert(indices.end(), {a, b, a + 1, a + 1, b, b + 1});
        }
    }
    return Mesh(std::move(verts), std::move(indices));
}

inline Mesh make_cube(float half_size = 1.0f) {
    const float h = half_size;
    std::vector<Vertex> verts;
    std::vector<std::uint32_t> indices;

    struct Face {
        Vec3f normal;
        Vec3f corners[4];
    };
    const Face faces[6] = {
        {{0, 0, 1},  {{-h, -h, h},  {h, -h, h},  {h, h, h},  {-h, h, h}}},
        {{0, 0, -1}, {{h, -h, -h}, {-h, -h, -h}, {-h, h, -h}, {h, h, -h}}},
        {{1, 0, 0},  {{h, -h, h},  {h, -h, -h}, {h, h, -h}, {h, h, h}}},
        {{-1, 0, 0}, {{-h, -h, -h}, {-h, -h, h}, {-h, h, h}, {-h, h, -h}}},
        {{0, 1, 0},  {{-h, h, h},  {h, h, h},   {h, h, -h}, {-h, h, -h}}},
        {{0, -1, 0}, {{-h, -h, -h}, {h, -h, -h}, {h, -h, h}, {-h, -h, h}}},
    };
    const Vec2f uvs[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

    for (const Face& f : faces) {
        const std::uint32_t base = static_cast<std::uint32_t>(verts.size());
        for (int i = 0; i < 4; ++i) {
            Vertex v;
            v.position = f.corners[i];
            v.normal = f.normal;
            v.uv = uvs[i];
            v.color = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
            verts.push_back(v);
        }
        indices.insert(indices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }
    return Mesh(std::move(verts), std::move(indices));
}

inline Mesh make_quad(float half_size = 1.0f) {
    const float h = half_size;
    std::vector<Vertex> verts(4);
    const Vec2f positions[4] = {{-h, -h}, {h, -h}, {h, h}, {-h, h}};
    const Vec2f uvs[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    for (int i = 0; i < 4; ++i) {
        verts[i].position = Vec3f(positions[i].x, positions[i].y, 0.0f);
        verts[i].normal = Vec3f(0.0f, 0.0f, 1.0f);
        verts[i].uv = uvs[i];
    }
    return Mesh(std::move(verts), {0, 1, 2, 0, 2, 3});
}

inline Mesh make_plane(float size = 10.0f, int divisions = 16, float tiles = 1.0f) {
    std::vector<Vertex> verts;
    std::vector<std::uint32_t> indices;
    verts.reserve(static_cast<std::size_t>(divisions + 1) * static_cast<std::size_t>(divisions + 1));

    for (int z = 0; z <= divisions; ++z) {
        for (int x = 0; x <= divisions; ++x) {
            const float u = static_cast<float>(x) / static_cast<float>(divisions);
            const float v = static_cast<float>(z) / static_cast<float>(divisions);
            Vertex vert;
            vert.position = Vec3f((u - 0.5f) * size, 0.0f, (v - 0.5f) * size);
            vert.normal = Vec3f(0.0f, 1.0f, 0.0f);
            vert.uv = Vec2f(u * tiles, v * tiles);
            verts.push_back(vert);
        }
    }

    const std::uint32_t stride = static_cast<std::uint32_t>(divisions + 1);
    for (std::uint32_t z = 0; z < static_cast<std::uint32_t>(divisions); ++z) {
        for (std::uint32_t x = 0; x < static_cast<std::uint32_t>(divisions); ++x) {
            const std::uint32_t a = z * stride + x;
            const std::uint32_t b = a + 1;
            const std::uint32_t c = a + stride;
            const std::uint32_t d = c + 1;
            indices.insert(indices.end(), {a, c, b, b, c, d});
        }
    }
    return Mesh(std::move(verts), std::move(indices));
}
