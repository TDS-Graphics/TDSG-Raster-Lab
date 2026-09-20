#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "core/camera.h"
#include "core/primitives.h"
#include "core/scene.h"
#include "image/framebuffer.h"
#include "image/image.h"
#include "io/obj_loader.h"
#include "math/transform.h"
#include "shaders/builtin.h"

struct Options {
    std::string obj_path;
    std::string texture_path;
    std::string shader_name = "phong";
    std::string out_path = "out.tga";
    int width = 1280;
    int height = 1024;
    int frames = 1;
    float fov = 60.0f;
    float yaw = 30.0f;
    float pitch = 15.0f;
    bool no_clip = false;
    bool help = false;
};

static void print_usage(const char* exe) {
    std::cout <<
        "usage: " << exe << " [options]\n"
        "  (no --obj)        render the built in hall, camera flying down it\n"
        "  --obj <path>      render a single Wavefront OBJ mesh instead\n"
        "  --tex <path>      diffuse texture for --obj, TGA\n"
        "  --shader <name>   flat | normal | texture | phong | depth   (default: phong)\n"
        "  --out <path>      output image (default: out.tga)\n"
        "  --width <n>       image width  (default: 800)\n"
        "  --height <n>      image height (default: 800)\n"
        "  --frames <n>      render n frames (default: 1)\n"
        "  --fov <deg>       vertical field of view (default: 60)\n"
        "  --yaw <deg>       camera yaw for --obj   (default: 30)\n"
        "  --pitch <deg>     camera pitch for --obj (default: 15)\n"
        "  --no-clip         skip Sutherland-Hodgman clipping\n"
        "  --help\n";
}

static bool next_value(int& i, int argc, char** argv, std::string& out) {
    if (i + 1 >= argc) return false;
    out = argv[++i];
    return true;
}

static bool parse_args(int argc, char** argv, Options& opt) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") { opt.help = true; }
        else if (arg == "--obj") { if (!next_value(i, argc, argv, opt.obj_path)) return false; }
        else if (arg == "--tex") { if (!next_value(i, argc, argv, opt.texture_path)) return false; }
        else if (arg == "--shader") { if (!next_value(i, argc, argv, opt.shader_name)) return false; }
        else if (arg == "--out") { if (!next_value(i, argc, argv, opt.out_path)) return false; }
        else if (arg == "--width") { opt.width = std::atoi(argv[++i]); }
        else if (arg == "--height") { opt.height = std::atoi(argv[++i]); }
        else if (arg == "--frames") { opt.frames = std::atoi(argv[++i]); }
        else if (arg == "--fov") { opt.fov = static_cast<float>(std::atof(argv[++i])); }
        else if (arg == "--yaw") { opt.yaw = static_cast<float>(std::atof(argv[++i])); }
        else if (arg == "--pitch") { opt.pitch = static_cast<float>(std::atof(argv[++i])); }
        else if (arg == "--no-clip") { opt.no_clip = true; }
        else {
            std::cerr << "unknown option: " << arg << "\n";
            return false;
        }
    }
    return true;
}

static std::shared_ptr<Shader> make_shader(const std::string& name) {
    if (name == "flat") return std::make_shared<FlatShader>();
    if (name == "normal") return std::make_shared<NormalShader>();
    if (name == "texture") return std::make_shared<TextureShader>();
    if (name == "depth") return std::make_shared<DepthShader>();
    if (name == "phong") return std::make_shared<PhongShader>();

    // Shader fallback
    std::cerr << "unknown shader '" << name << "', falling back to phong\n";
    return std::make_shared<PhongShader>();
}

static Mat4f fit_transform(const Mesh& mesh) {
    const Bounds3f bounds = mesh.bounds();
    const float radius = bounds.radius();
    const float s = radius > 0.0f ? 1.0f / radius : 1.0f;
    const Vec3f c = bounds.center();
    return scale(Vec3f(s, s, s)) * translate(Vec3f(-c.x, -c.y, -c.z));
}

static Texture make_checker(const Color& dark, const Color& light) {
    const int kSize = 64;
    const int kCells = 2;
    Image image(kSize, kSize, Image::RGBA);
    for (int y = 0; y < kSize; ++y)
        for (int x = 0; x < kSize; ++x)
            image.set(x, y, ((x * kCells / kSize) + (y * kCells / kSize)) % 2 == 0 ? dark : light);

    Texture texture(std::move(image));
    texture.filter = Texture::Filter::Nearest;
    return texture;
}

static std::string frame_path(const std::string& out, int frame) {
    const std::size_t dot = out.rfind('.');
    const std::string stem = dot == std::string::npos ? out : out.substr(0, dot);
    const std::string ext = dot == std::string::npos ? std::string(".tga") : out.substr(dot);
    char suffix[16];
    std::snprintf(suffix, sizeof(suffix), "_%04d", frame);
    return stem + suffix + ext;
}

static int run_hall(const Options& opt) {
    std::cout << "";

    const float kHalfWidth = 7.0f;
    const float kCeiling = 7.0f;
    const float kLength = 90.0f;
    const float kPi = 3.1415926535f;

    Scene scene;
    std::vector<Mesh> meshes;

    meshes.push_back(make_plane(kLength, 30, 30.0f));
    meshes.push_back(make_plane(kLength, 10, 15.0f));
    meshes.push_back(make_plane(kLength, 14, 15.0f));
    meshes.push_back(make_cube(1.0f));
    meshes.push_back(make_sphere(1.0f, 32, 20));
    const Mesh& floor_mesh = meshes[0];
    const Mesh& ceiling_mesh = meshes[1];
    const Mesh& wall_mesh = meshes[2];
    const Mesh& box_mesh = meshes[3];
    const Mesh& ball_mesh = meshes[4];

    const std::shared_ptr<Shader> shader = make_shader(opt.shader_name);
    const Texture checker = make_checker(Color(0.22f, 0.23f, 0.27f), Color(0.72f, 0.73f, 0.78f));

    Material stone = Material::textured(&checker, 16.0f);
    stone.specular = Vec3f(0.10f, 0.10f, 0.10f);

    scene.add(floor_mesh, shader, translate(Vec3f(0.0f, 0.0f, 0.0f))).material = stone;
    scene.add(ceiling_mesh, shader, translate(Vec3f(0.0f, kCeiling, 0.0f)) * rotate_z(kPi)).material = stone;
    scene.add(wall_mesh, shader, translate(Vec3f(-kHalfWidth, 0.0f, 0.0f)) * rotate_z(-kPi / 2)).material = stone;
    scene.add(wall_mesh, shader, translate(Vec3f(kHalfWidth, 0.0f, 0.0f)) * rotate_z(kPi / 2)).material = stone;

    for (int i = 0; i < 9; ++i) {
        const float z = 24.0f - static_cast<float>(i) * 6.0f;
        for (int side = -1; side <= 1; side += 2) {
            Object& column = scene.add(box_mesh, shader,
                                       translate(Vec3f(4.0f * static_cast<float>(side), 3.5f, z)) *
                                           scale(Vec3f(0.45f, 3.5f, 0.45f)));
            column.material.diffuse = Vec3f(0.66f, 0.62f, 0.56f);
            column.material.specular = Vec3f(0.20f, 0.20f, 0.20f);
            column.material.shininess = 24.0f;
        }
    }

    const Vec3f ball_colors[5] = {{0.85f, 0.22f, 0.18f},
                                  {0.20f, 0.42f, 0.85f},
                                  {0.90f, 0.70f, 0.18f},
                                  {0.22f, 0.72f, 0.38f},
                                  {0.78f, 0.78f, 0.84f}};
    for (int i = 0; i < 5; ++i) {
        const float radius = 1.20f - 0.12f * static_cast<float>(i);
        const float x = (i % 2 == 0) ? -2.6f : 2.6f;
        const float z = 15.0f - static_cast<float>(i) * 8.0f;

        Object& ball = scene.add(ball_mesh, shader,
                                 translate(Vec3f(x, radius, z)) * scale(Vec3f(radius, radius, radius)));
        ball.material.diffuse = ball_colors[i];
        ball.material.specular = Vec3f(0.85f, 0.85f, 0.85f);
        ball.material.shininess = 96.0f;
    }

    Object& crate_a = scene.add(box_mesh, shader, translate(Vec3f(-3.6f, 0.6f, 7.0f)) * rotate_y(0.4f));
    crate_a.material.diffuse = Vec3f(0.55f, 0.38f, 0.22f);
    crate_a.material.shininess = 16.0f;

    Object& crate_b = scene.add(box_mesh, shader,
                                translate(Vec3f(3.4f, 0.6f, -1.0f)) * rotate_y(-0.7f) * scale(Vec3f(1.0f, 0.6f, 1.0f)));
    crate_b.material.diffuse = Vec3f(0.42f, 0.30f, 0.20f);
    crate_b.material.shininess = 16.0f;

    scene.ambient_light = Vec3f(0.55f, 0.60f, 0.72f);
    scene.lights.push_back({Vec3f(0.0f, 4.5f, 20.0f), Vec3f(46.0f, 42.0f, 34.0f)});
    scene.lights.push_back({Vec3f(-4.5f, 4.0f, 0.0f), Vec3f(16.0f, 22.0f, 34.0f)});
    scene.lights.push_back({Vec3f(3.5f, 3.5f, -26.0f), Vec3f(40.0f, 18.0f, 10.0f)});

    Camera camera;
    camera.fov_y_degrees = opt.fov;
    camera.aspect = static_cast<float>(opt.width) / static_cast<float>(opt.height);
    camera.near_plane = 0.1f;
    camera.far_plane = 400.0f;

    Framebuffer framebuffer(opt.width, opt.height);
    for (int frame = 0; frame < opt.frames; ++frame) {
        const float t = opt.frames > 1 ? static_cast<float>(frame) / static_cast<float>(opt.frames - 1) : 0.0f;
        camera.position = Vec3f(1.5f * std::sin(t * 4.0f), 1.9f, 26.0f - 46.0f * t);
        camera.target = Vec3f(0.0f, 1.6f, camera.position.z - 12.0f);

        framebuffer.clear(Color(0.04f, 0.05f, 0.07f, 1.0f));
        RenderOptions options;
        options.clip_frustum = !opt.no_clip;
        const RenderStats stats = render(scene, camera, framebuffer, options);

        const std::string path = opt.frames > 1 ? frame_path(opt.out_path, frame) : opt.out_path;
        if (!framebuffer.save(path)) {
            std::cerr << "could not write '" << path << "'\n";
            return 1;
        }
        std::cout << "wrote " << path << "  (" << stats.objects << " objects, " << stats.triangles
                  << " triangles, " << stats.clipped << " clipped, " << stats.fragments << " fragments)\n";
    }
    return 0;
}

static int run_mesh(const Options& opt) {
    ObjLoadResult loaded = load_obj(opt.obj_path);
    if (!loaded.ok) {
        std::cerr << "failed to load '" << opt.obj_path << "': " << loaded.error << "\n";
        return 1;
    }
    Mesh mesh = std::move(loaded.mesh);
    std::cout << "loaded " << opt.obj_path << ": " << mesh.vertex_count() << " vertices, "
              << mesh.triangle_count() << " triangles\n";

    Texture texture;
    if (!opt.texture_path.empty()) {
        texture = Texture::load(opt.texture_path);
        if (!texture.valid()) {
            std::cerr << "warning: could not read texture '" << opt.texture_path << "'\n";
        } else {
            std::cout << "loaded texture " << opt.texture_path << " (" << texture.width() << "x"
                      << texture.height() << ")\n";
        }
    }

    Scene scene;
    scene.ambient_light = Vec3f(2.0f, 2.0f, 2.2f);
    scene.lights.push_back({Vec3f(4.0f, 4.0f, 4.0f), Vec3f(40.0f, 40.0f, 40.0f)});
    scene.lights.push_back({Vec3f(-4.0f, 2.0f, 2.0f), Vec3f(15.0f, 15.0f, 15.0f)});

    const Mat4f fit = fit_transform(mesh);
    Object& object = scene.add(mesh, make_shader(opt.shader_name));
    object.material = Material::textured(texture.valid() ? &texture : nullptr, 80.0f);
    object.material.specular = Vec3f(0.9f, 0.9f, 0.9f);

    Camera camera;
    camera.fov_y_degrees = opt.fov;
    camera.aspect = static_cast<float>(opt.width) / static_cast<float>(opt.height);
    camera.target = Vec3f(0.0f, 0.0f, 0.0f);
    camera.orbit(opt.yaw, opt.pitch, 3.0f);
    camera.frame(Vec3f(0.0f, 0.0f, 0.0f), 1.0f);

    Framebuffer framebuffer(opt.width, opt.height);
    for (int frame = 0; frame < opt.frames; ++frame) {
        const float degrees = 360.0f * static_cast<float>(frame) / static_cast<float>(opt.frames);
        object.transform = rotate_y(degrees * 3.1415926535f / 180.0f) * fit;

        framebuffer.clear(Color(0.05f, 0.06f, 0.09f, 1.0f));
        const RenderStats stats = render(scene, camera, framebuffer);

        const std::string path = opt.frames > 1 ? frame_path(opt.out_path, frame) : opt.out_path;
        if (!framebuffer.save(path)) {
            std::cerr << "could not write '" << path << "'\n";
            return 1;
        }
        std::cout << "wrote " << path << "  (" << stats.triangles << " triangles, " << stats.clipped
                  << " clipped, " << stats.fragments << " fragments)\n";
    }
    return 0;
}

int main(int argc, char** argv) {
    Options opt;
    if (!parse_args(argc, argv, opt)) {
        print_usage(argv[0]);
        return 1;
    }
    if (opt.help) {
        print_usage(argv[0]);
        return 0;
    }
    if (opt.width <= 0 || opt.height <= 0 || opt.frames <= 0) {
        std::cerr << "width, height and frames must be positive\n";
        return 1;
    }

    // actual rendering
    return opt.obj_path.empty() ? run_hall(opt) : run_mesh(opt);
}
