#include "image/framebuffer.h"

#include <algorithm>

Framebuffer::Framebuffer(int width, int height) { resize(width, height); }

void Framebuffer::resize(int width, int height) {
    w_ = std::max(0, width);
    h_ = std::max(0, height);
    color_.assign(w_ * h_, Color(0, 0, 0, 1));
    depth_.assign(w_ * h_, 1.0f);
}

void Framebuffer::clear(const Color& c, float depth_value) {
    clear_color(c);
    clear_depth(depth_value);
}

void Framebuffer::clear_color(const Color& c) {
    std::fill(color_.begin(), color_.end(), c);
}

void Framebuffer::clear_depth(float depth_value) {
    std::fill(depth_.begin(), depth_.end(), depth_value);
}

Image Framebuffer::to_image() const {
    Image img(w_, h_, Image::RGBA);
    for (int y = 0; y < h_; ++y)
        for (int x = 0; x < w_; ++x) img.set(x, y, color(x, y).clamped().srgb());
    return img;
}

bool Framebuffer::save(const std::string& filename, bool rle) const {
    return to_image().save_tga(filename, rle);
}
