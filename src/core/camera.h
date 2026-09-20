#pragma once

#include <cmath>
#include <numbers>

#include "math/transform.h"

class Camera {
public:
    Vec3f position{0.0f, 0.0f, 3.0f};
    Vec3f target{0.0f, 0.0f, 0.0f};
    Vec3f up{0.0f, 1.0f, 0.0f};

    float fov_y_degrees = 45.0f;
    float near_plane = 0.1f;
    float far_plane = 100.0f;
    float aspect = 1.0f;

    Mat4f view() const { return look_at(position, target, up); }

    Mat4f projection() const {
        const float fov = fov_y_degrees * std::numbers::pi_v<float> / 180.0f;
        return perspective(fov, aspect, near_plane, far_plane);
    }

    Mat4f view_projection() const { return projection() * view(); }

    float frame(const Vec3f& center, float radius, float margin = 1.25f) {
        target = center;
        const float half_fov = fov_y_degrees * std::numbers::pi_v<float> / 360.0f;
        const float distance = margin * radius / std::tan(half_fov);
        const Vec3f dir = normalize(position - target);
        position = target + dir * distance;
        near_plane = std::max(0.01f, distance - radius * 2.0f);
        far_plane = distance + radius * 4.0f + 1.0f;
        return distance;
    }

    void orbit(float yaw_degrees, float pitch_degrees, float distance) {
        const float yaw = yaw_degrees * std::numbers::pi_v<float> / 180.0f;
        const float pitch = pitch_degrees * std::numbers::pi_v<float> / 180.0f;
        const Vec3f dir(std::cos(pitch) * std::sin(yaw), std::sin(pitch), std::cos(pitch) * std::cos(yaw));
        position = target + dir * distance;
    }
};
