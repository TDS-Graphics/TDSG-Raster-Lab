#include "core/rasterizer.h"

#include <algorithm>
#include <cmath>

static float edge(float ax, float ay, float bx, float by, float px, float py) {
    return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
}

static VSOutput clip_lerp(const VSOutput& a, const VSOutput& b, float t) {
    VSOutput out;
    out.clip_pos = a.clip_pos * (1.0f - t) + b.clip_pos * t;

    const float wa = a.clip_pos.w;
    const float wb = b.clip_pos.w;
    const float iwa = 1.0f / (std::abs(wa) > 1e-12f ? wa : (wa < 0.0f ? -1e-12f : 1e-12f));
    const float iwb = 1.0f / (std::abs(wb) > 1e-12f ? wb : (wb < 0.0f ? -1e-12f : 1e-12f));

    const float ia = iwa * (1.0f - t);
    const float ib = iwb * t;
    const float inv_w = ia + ib;
    out.vary = (a.vary * ia + b.vary * ib) * (1.0f / inv_w);
    return out;
}

struct ClipPlane {
    float a, b, c, d;
};

static const ClipPlane kFrustumPlanes[6] = {
    { 0.0f,  0.0f,  1.0f, 1.0f},
    { 0.0f,  0.0f, -1.0f, 1.0f},
    { 1.0f,  0.0f,  0.0f, 1.0f},
    {-1.0f,  0.0f,  0.0f, 1.0f},
    { 0.0f,  1.0f,  0.0f, 1.0f},
    { 0.0f, -1.0f,  0.0f, 1.0f},
};

static float plane_distance(const ClipPlane& plane, const Vec4f& clip_pos) {
    return plane.a * clip_pos.x + plane.b * clip_pos.y + plane.c * clip_pos.z + plane.d * clip_pos.w;
}

static int clip_polygon(const VSOutput* in, int n, const ClipPlane& plane, VSOutput* out) {
    int m = 0;
    for (int i = 0; i < n; ++i) {
        const VSOutput& cur = in[i];
        const VSOutput& nxt = in[(i + 1) % n];
        const float d_cur = plane_distance(plane, cur.clip_pos);
        const float d_nxt = plane_distance(plane, nxt.clip_pos);
        const bool cur_in = d_cur >= 0.0f;
        const bool nxt_in = d_nxt >= 0.0f;

        if (cur_in) out[m++] = cur;
        if (cur_in != nxt_in) out[m++] = clip_lerp(cur, nxt, d_cur / (d_cur - d_nxt));
    }
    return m;
}

void Rasterizer::emit_triangle(const VSOutput& a, const VSOutput& b, const VSOutput& c) {
    VSOutput buffer[2][12];
    buffer[0][0] = a;
    buffer[0][1] = b;
    buffer[0][2] = c;
    int n = 3;
    int src = 0;
    bool clipped = false;

    if (clip_frustum) {
        for (const ClipPlane& plane : kFrustumPlanes) {
            const int before = n;
            n = clip_polygon(buffer[src], n, plane, buffer[1 - src]);
            src = 1 - src;
            if (n != before) clipped = true;
            if (n < 3) break;
        }
    }

    if (clipped) ++clipped_triangles;
    if (n < 3) return;

    for (int i = 1; i + 1 < n; ++i) {
        rasterize(buffer[src][0], buffer[src][i], buffer[src][i + 1]);
    }
    drawn_triangles += n - 2;
}

void Rasterizer::rasterize(const VSOutput& a, const VSOutput& b, const VSOutput& c) {
    if (shader_ == nullptr || target_.empty()) return;

    const int vw = viewport_width >= 0 ? viewport_width : target_.width();
    const int vh = viewport_height >= 0 ? viewport_height : target_.height();
    if (vw <= 0 || vh <= 0) return;

    const VSOutput* tri[3] = {&a, &b, &c};
    ScreenVertex sv[3];
    for (int i = 0; i < 3; ++i) {
        const Vec4f& p = tri[i]->clip_pos;
        const Vec3f ndc = p.xyz() / p.w;
        sv[i].x = (ndc.x * 0.5f + 0.5f) * static_cast<float>(vw) + static_cast<float>(viewport_x);
        sv[i].y = (1.0f - (ndc.y * 0.5f + 0.5f)) * static_cast<float>(vh) + static_cast<float>(viewport_y);
        sv[i].z = ndc.z * 0.5f + 0.5f;
        sv[i].inv_w = 1.0f / p.w;
        sv[i].vary = tri[i]->vary;
    }

    const float area = edge(sv[0].x, sv[0].y, sv[1].x, sv[1].y, sv[2].x, sv[2].y);
    float kAreaEps = 1e-9f;
    if (std::abs(area) < kAreaEps) return;
    if (cull_mode == CullMode::Back && area > 0.0f) return;
    if (cull_mode == CullMode::Front && area < 0.0f) return;

    const float inv_area = 1.0f / area;

    const int min_x = static_cast<int>(std::max(0.0f, std::floor(std::min({sv[0].x, sv[1].x, sv[2].x}))));
    const int max_x = static_cast<int>(
        std::min(static_cast<float>(target_.width() - 1), std::ceil(std::max({sv[0].x, sv[1].x, sv[2].x}))));
    const int min_y = static_cast<int>(std::max(0.0f, std::floor(std::min({sv[0].y, sv[1].y, sv[2].y}))));
    const int max_y = static_cast<int>(
        std::min(static_cast<float>(target_.height() - 1), std::ceil(std::max({sv[0].y, sv[1].y, sv[2].y}))));
    if (min_x > max_x || min_y > max_y) return;

    const Shader* shader = shader_;
    const Uniforms& uniforms = uniforms_;

    std::size_t shaded = 0;

#ifdef TDSG_OPENMP
#pragma omp parallel for schedule(dynamic, 4) reduction(+ : shaded)
#endif
    for (int y = min_y; y <= max_y; ++y) {
        const float py = static_cast<float>(y) + 0.5f;
        for (int x = min_x; x <= max_x; ++x) {
            const float px = static_cast<float>(x) + 0.5f;

            const float e0 = edge(sv[1].x, sv[1].y, sv[2].x, sv[2].y, px, py);
            const float e1 = edge(sv[2].x, sv[2].y, sv[0].x, sv[0].y, px, py);
            const float e2 = edge(sv[0].x, sv[0].y, sv[1].x, sv[1].y, px, py);
            if (e0 * area < 0.0f || e1 * area < 0.0f || e2 * area < 0.0f) continue;

            const float l0 = e0 * inv_area;
            const float l1 = e1 * inv_area;
            const float l2 = e2 * inv_area;

            const float z = l0 * sv[0].z + l1 * sv[1].z + l2 * sv[2].z;
            if (depth_test && z >= target_.depth(x, y)) continue;

            const float k0 = l0 * sv[0].inv_w;
            const float k1 = l1 * sv[1].inv_w;
            const float k2 = l2 * sv[2].inv_w;
            const float inv_k = 1.0f / (k0 + k1 + k2);
            Fragment frag = (sv[0].vary * k0 + sv[1].vary * k1 + sv[2].vary * k2) * inv_k;
            frag.depth = z;

            if (depth_write) target_.depth(x, y) = z;
            target_.color(x, y) = shader->fragment(frag, uniforms);
            ++shaded;
        }
    }

    shaded_fragments += shaded;
}

void Rasterizer::draw(const Mesh& mesh) {
    if (shader_ == nullptr) return;

    // Fill vertex buffer and index buffer
    const std::vector<std::uint32_t>& indices = mesh.indices();
    const std::vector<Vertex>& vertices = mesh.vertices();

    for (std::size_t i = 0; i + 2 < indices.size(); i += 3) {
        VSOutput out[3];
        bool valid = true;
        for (int k = 0; k < 3; ++k) {
            const std::uint32_t index = indices[i + k];
            if (index >= vertices.size()) { valid = false; break; }
            out[k] = shader_->vertex(vertices[index], uniforms_);
        }
        if (!valid) continue;
        emit_triangle(out[0], out[1], out[2]);
    }
}
