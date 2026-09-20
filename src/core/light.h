#pragma once

#include "core/texture.h"
#include "math/vec.h"

struct Light {
    Vec3f position{0.0f, 5.0f, 5.0f};
    Vec3f intensity{1.0f, 1.0f, 1.0f};
};

struct Material {
    Vec3f ambient{0.02f, 0.02f, 0.02f};
    Vec3f diffuse{1.0f, 1.0f, 1.0f};
    Vec3f specular{0.5f, 0.5f, 0.5f};
    float shininess = 32.0f;

    const Texture* texture = nullptr;

    static Material textured(const Texture* tex, float shininess = 150.0f) {
        Material m;
        m.shininess = shininess;
        m.texture = tex;
        return m;
    }
};
