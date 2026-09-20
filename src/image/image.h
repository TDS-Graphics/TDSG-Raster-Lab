#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

#include "image/color.h"

class Image {
public:
    enum Format : int { Grayscale = 1, GrayAlpha = 2, RGB = 3, RGBA = 4 };

    Image() {}
    Image(int w, int h, int channels = RGBA);
    Image(int w, int h, const Color& fill, int channels = RGBA);

    static Image load_tga(const std::string& filename);
    bool save_tga(const std::string& filename, bool rle = true) const;

    bool valid() const { return w_ > 0 && h_ > 0 && !data_.empty(); }
    int  width() const { return w_; }
    int  height() const { return h_; }
    int  channels() const { return channels_; }
    std::size_t pixel_count() const { return w_ * h_; }

    Color get(int x, int y) const;
    void  set(int x, int y, const Color& c);
    void  fill(const Color& c);

    void flip_vertically();
    void flip_horizontally();

    std::vector<std::uint8_t>&       data()       { return data_; }
    const std::vector<std::uint8_t>& data() const { return data_; }

private:
    std::size_t offset(int x, int y) const {
        return (x + y * w_) * channels_;
    }

    bool load_rle(std::istream& in);
    bool save_rle(std::ostream& out) const;

    int w_ = 0;
    int h_ = 0;
    int channels_ = 0;
    std::vector<std::uint8_t> data_;
};
