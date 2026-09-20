#pragma once

#include <cmath>
#include <string>

#include "image/image.h"
#include "math/vec.h"

class Texture {
public:
    enum class Wrap { Repeat, Clamp };
    enum class Filter { Nearest, Bilinear };

    Texture() {}
    explicit Texture(Image image) : image_(std::move(image)) {}

    static Texture load(const std::string& filename) { return Texture(Image::load_tga(filename)); }

    bool valid() const { return image_.valid(); }
    int width() const { return image_.width(); }
    int height() const { return image_.height(); }
    const Image& image() const { return image_; }

    Color sample(const Vec2f& uv) const {
        if (!valid()) return Color(1.0f, 0.0f, 1.0f, 1.0f);

        const float w = static_cast<float>(width());
        const float h = static_cast<float>(height());
        const float fx = uv.x * w - 0.5f;
        const float fy = (1.0f - uv.y) * h - 0.5f;

        if (filter == Filter::Nearest) {
            return image_.get(wrap_index(static_cast<int>(std::floor(fx + 0.5f)), width()),
                              wrap_index(static_cast<int>(std::floor(fy + 0.5f)), height()));
        }

        const int x0 = static_cast<int>(std::floor(fx));
        const int y0 = static_cast<int>(std::floor(fy));
        const float tx = fx - static_cast<float>(x0);
        const float ty = fy - static_cast<float>(y0);

        const Color c00 = image_.get(wrap_index(x0, width()), wrap_index(y0, height()));
        const Color c10 = image_.get(wrap_index(x0 + 1, width()), wrap_index(y0, height()));
        const Color c01 = image_.get(wrap_index(x0, width()), wrap_index(y0 + 1, height()));
        const Color c11 = image_.get(wrap_index(x0 + 1, width()), wrap_index(y0 + 1, height()));

        const Color top = c00 * (1.0f - tx) + c10 * tx;
        const Color bottom = c01 * (1.0f - tx) + c11 * tx;
        return top * (1.0f - ty) + bottom * ty;
    }

    Color texel(int x, int y) const { return image_.get(x, y); }

    Wrap wrap = Wrap::Repeat;
    Filter filter = Filter::Bilinear;

private:
    int wrap_index(int i, int n) const {
        if (n <= 0) return 0;
        if (wrap == Wrap::Clamp) return i < 0 ? 0 : (i >= n ? n - 1 : i);
        i %= n;
        return i < 0 ? i + n : i;
    }

    Image image_;
};
