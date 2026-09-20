#include "core/scene.h"

#include "core/rasterizer.h"

RenderStats render(const Scene& scene, const Camera& camera, Framebuffer& target, RenderOptions options) {
    Rasterizer rasterizer(target);
    rasterizer.clip_frustum = options.clip_frustum;
    RenderStats stats;

    const Mat4f view = camera.view();
    const Mat4f projection = camera.projection();

    for (const Object& object : scene.objects) {
        if (!object.visible || object.mesh == nullptr || !object.shader) continue;

        Uniforms uniforms;
        uniforms.model = object.transform;
        uniforms.view = view;
        uniforms.projection = projection;
        uniforms.camera_position = camera.position;
        uniforms.lights = scene.lights;
        uniforms.ambient_light = scene.ambient_light;
        uniforms.material = object.material;

        rasterizer.set_uniforms(uniforms);
        rasterizer.set_shader(object.shader.get());
        rasterizer.draw(*object.mesh);

        ++stats.objects;
        stats.triangles += object.mesh->triangle_count();
    }

    stats.clipped = rasterizer.clipped_triangles;
    stats.fragments = rasterizer.shaded_fragments;
    return stats;
}
