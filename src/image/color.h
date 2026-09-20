#pragma once

#include <cmath>
#include <cstdint>

#include "math/vec.h"

inline std::uint8_t to_u8(float v) {
    return static_cast<std::uint8_t>(v <= 0.0f ? 0.0f : (v >= 1.0f ? 255.0f : v * 255.0f + 0.5f));
}

struct Color {
    float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;

    Color() {}
    Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
    explicit Color(const Vec3f& v, float a_ = 1.0f) : r(v.x), g(v.y), b(v.z), a(a_) {}

    static Color from_rgb8(int r_, int g_, int b_, int a_ = 255) {
        return Color(static_cast<float>(r_) / 255.0f,
                     static_cast<float>(g_) / 255.0f,
                     static_cast<float>(b_) / 255.0f,
                     static_cast<float>(a_) / 255.0f);
    }

    static Color from_hex(std::uint32_t hex, bool with_alpha = false) {
        const float a = with_alpha ? static_cast<float>((hex >> 24) & 0xFF) / 255.0f : 1.0f;
        return Color(static_cast<float>((hex >> 16) & 0xFF) / 255.0f,
                     static_cast<float>((hex >> 8) & 0xFF) / 255.0f,
                     static_cast<float>(hex & 0xFF) / 255.0f,
                     a);
    }

    std::uint32_t to_rgba8() const {
        return static_cast<std::uint32_t>(to_u8(r)) |
               (static_cast<std::uint32_t>(to_u8(g)) << 8) |
               (static_cast<std::uint32_t>(to_u8(b)) << 16) |
               (static_cast<std::uint32_t>(to_u8(a)) << 24);
    }

    Vec3f rgb() const { return Vec3f(r, g, b); }

    Color operator+(const Color& o) const { return Color(r + o.r, g + o.g, b + o.b, a + o.a); }
    Color operator-(const Color& o) const { return Color(r - o.r, g - o.g, b - o.b, a - o.a); }
    Color operator*(float s) const { return Color(r * s, g * s, b * s, a * s); }
    Color operator*(const Color& o) const { return Color(r * o.r, g * o.g, b * o.b, a * o.a); }
    Color& operator+=(const Color& o) { r += o.r; g += o.g; b += o.b; a += o.a; return *this; }
    Color& operator*=(float s) { r *= s; g *= s; b *= s; a *= s; return *this; }

    Color clamped() const {
        return Color(r < 0 ? 0 : (r > 1 ? 1 : r),
                     g < 0 ? 0 : (g > 1 ? 1 : g),
                     b < 0 ? 0 : (b > 1 ? 1 : b),
                     a < 0 ? 0 : (a > 1 ? 1 : a));
    }

    static float to_srgb(float v) {
        if (v <= 0.0f) return 0.0f;
        if (v >= 1.0f) return 1.0f;
        return v <= 0.0031308f ? v * 12.92f : 1.055f * std::pow(v, 1.0f / 2.4f) - 0.055f;
    }

    Color srgb() const { return Color(to_srgb(r), to_srgb(g), to_srgb(b), a); }

    float luminance() const { return 0.2126f * r + 0.7152f * g + 0.0722f * b; }

    static Color black() { return Color(0, 0, 0, 1); }
    static Color white() { return Color(1, 1, 1, 1); }
    static Color red()   { return Color(1, 0, 0, 1); }
    static Color green() { return Color(0, 1, 0, 1); }
    static Color blue()  { return Color(0, 0, 1, 1); }
    static Color magenta() { return Color(1, 0, 1, 1); }
};

inline Vec3f to_vec3(const Color& c) { return Vec3f(c.r, c.g, c.b); }
