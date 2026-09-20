#pragma once

#include <cstdint>
#include <vector>

#include "core/geometry.h"

class Mesh {
public:
    Mesh() {}
    Mesh(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices)
        : vertices_(std::move(vertices)), indices_(std::move(indices)) {}

    const std::vector<Vertex>& vertices() const { return vertices_; }
    const std::vector<std::uint32_t>& indices() const { return indices_; }

    std::vector<Vertex>& vertices() { return vertices_; }
    std::vector<std::uint32_t>& indices() { return indices_; }

    int vertex_count() const { return static_cast<int>(vertices_.size()); }
    int index_count() const { return static_cast<int>(indices_.size()); }
    int triangle_count() const { return static_cast<int>(indices_.size() / 3); }

    Bounds3f bounds() const {
        Bounds3f b;
        for (const Vertex& v : vertices_) b.expand(v.position);
        return b;
    }

    void compute_normals() {
        for (Vertex& v : vertices_) v.normal = Vec3f(0.0f, 0.0f, 0.0f);
        for (std::size_t i = 0; i + 2 < indices_.size(); i += 3) {
            Vertex& a = vertices_[indices_[i]];
            Vertex& b = vertices_[indices_[i + 1]];
            Vertex& c = vertices_[indices_[i + 2]];
            const Vec3f n = cross(b.position - a.position, c.position - a.position);
            a.normal += n;
            b.normal += n;
            c.normal += n;
        }
        for (Vertex& v : vertices_) {
            const float len = length(v.normal);
            v.normal = len > 0.0f ? v.normal / len : Vec3f(0.0f, 0.0f, 1.0f);
        }
    }

private:
    std::vector<Vertex> vertices_;
    std::vector<std::uint32_t> indices_;
};
