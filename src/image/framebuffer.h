#pragma once

#include <string>
#include <vector>

#include "image/color.h"
#include "image/image.h"
#include "math/vec.h"

class Framebuffer {
public:
    Framebuffer() {}
    Framebuffer(int width, int height);

    int width() const { return w_; }
    int height() const { return h_; }
    Vec2i size() const { return Vec2i(w_, h_); }
    bool empty() const { return w_ <= 0 || h_ <= 0; }

    void resize(int width, int height);

    void clear(const Color& c = Color(0, 0, 0, 1), float depth_value = 1.0f);
    void clear_color(const Color& c = Color(0, 0, 0, 1));
    void clear_depth(float depth_value = 1.0f);

    bool contains(int x, int y) const { return x >= 0 && y >= 0 && x < w_ && y < h_; }

    Color&       color(int x, int y)       { return color_[index(x, y)]; }
    const Color& color(int x, int y) const { return color_[index(x, y)]; }
    float&       depth(int x, int y)       { return depth_[index(x, y)]; }
    const float& depth(int x, int y) const { return depth_[index(x, y)]; }

    const std::vector<Color>& colors() const { return color_; }
    const std::vector<float>& depths() const { return depth_; }

    Image to_image() const;
    bool save(const std::string& filename, bool rle = true) const;

private:
    std::size_t index(int x, int y) const {
        return y * w_ + x;
    }

    int w_ = 0;
    int h_ = 0;
    std::vector<Color> color_;
    std::vector<float> depth_;
};
