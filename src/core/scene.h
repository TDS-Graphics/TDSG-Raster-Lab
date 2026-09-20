#pragma once

#include <memory>
#include <vector>

#include "core/camera.h"
#include "core/light.h"
#include "core/mesh.h"
#include "core/shader.h"
#include "image/framebuffer.h"
#include "math/mat.h"

struct Object {
    const Mesh* mesh = nullptr;
    Mat4f transform = Mat4f::identity();
    std::shared_ptr<Shader> shader;
    Material material;
    bool visible = true;
};

class Scene {
public:
    std::vector<Object> objects;
    std::vector<Light> lights;
    Vec3f ambient_light{0.05f, 0.05f, 0.05f};

    Object& add(const Mesh& mesh, std::shared_ptr<Shader> shader, const Mat4f& transform = Mat4f::identity()) {
        Object object;
        object.mesh = &mesh;
        object.transform = transform;
        object.shader = std::move(shader);
        objects.push_back(std::move(object));
        return objects.back();
    }

    void clear() { objects.clear(); }
};

struct RenderOptions {
    bool clip_frustum = true;
};

struct RenderStats {
    std::size_t objects = 0;
    std::size_t triangles = 0;
    std::size_t clipped = 0;
    std::size_t fragments = 0;
};

RenderStats render(const Scene& scene, const Camera& camera, Framebuffer& target,
                   RenderOptions options = {});
