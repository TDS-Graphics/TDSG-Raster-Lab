#include "image/image.h"

#include <cstring>
#include <fstream>
#include <istream>
#include <ostream>

#pragma pack(push, 1)
struct TgaHeader {
    std::uint8_t  id_length = 0;
    std::uint8_t  color_map_type = 0;
    std::uint8_t  image_type = 0;
    std::uint16_t color_map_first = 0;
    std::uint16_t color_map_length = 0;
    std::uint8_t  color_map_depth = 0;
    std::uint16_t x_origin = 0;
    std::uint16_t y_origin = 0;
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::uint8_t  bits_per_pixel = 0;
    std::uint8_t  image_descriptor = 0;
};
#pragma pack(pop)

std::uint8_t kOriginTopLeft = 0x20;
std::uint8_t kOriginRight   = 0x10;

std::uint8_t kTypeTrueColor     = 2;
std::uint8_t kTypeGray          = 3;
std::uint8_t kTypeRleTrueColor  = 10;
std::uint8_t kTypeRleGray       = 11;

std::uint8_t kMaxChunk = 128;

const std::uint8_t kFooter[18] = {'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O', 'N',
                                  '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'};

Image::Image(int w, int h, int channels)
    : w_(w), h_(h), channels_(channels),
      data_(w * h * channels, 0) {}

Image::Image(int w, int h, const Color& fill, int channels) : Image(w, h, channels) {
    this->fill(fill);
}

Color Image::get(int x, int y) const {
    if (!valid() || x < 0 || y < 0 || x >= w_ || y >= h_) return Color(0, 0, 0, 0);
    const std::uint8_t* p = data_.data() + offset(x, y);

    switch (channels_) {
        case Grayscale: {
            const float g = static_cast<float>(p[0]) / 255.0f;
            return Color(g, g, g, 1.0f);
        }
        case GrayAlpha: {
            const float g = static_cast<float>(p[0]) / 255.0f;
            return Color(g, g, g, static_cast<float>(p[1]) / 255.0f);
        }
        default: {
            const float b = static_cast<float>(p[0]) / 255.0f;
            const float g = static_cast<float>(p[1]) / 255.0f;
            const float r = static_cast<float>(p[2]) / 255.0f;
            const float a = channels_ >= 4 ? static_cast<float>(p[3]) / 255.0f : 1.0f;
            return Color(r, g, b, a);
        }
    }
}

void Image::set(int x, int y, const Color& c) {
    if (!valid() || x < 0 || y < 0 || x >= w_ || y >= h_) return;
    std::uint8_t* p = data_.data() + offset(x, y);

    if (channels_ == Grayscale) {
        p[0] = to_u8(c.luminance());
        return;
    }
    if (channels_ == GrayAlpha) {
        p[0] = to_u8(c.luminance());
        p[1] = to_u8(c.a);
        return;
    }
    p[0] = to_u8(c.b);
    p[1] = to_u8(c.g);
    p[2] = to_u8(c.r);
    if (channels_ >= 4) p[3] = to_u8(c.a);
}

void Image::fill(const Color& c) {
    for (int y = 0; y < h_; ++y)
        for (int x = 0; x < w_; ++x) set(x, y, c);
}

void Image::flip_horizontally() {
    const std::size_t stride = channels_;
    for (int y = 0; y < h_; ++y) {
        for (int x = 0; x < w_ / 2; ++x) {
            std::uint8_t* a = data_.data() + offset(x, y);
            std::uint8_t* b = data_.data() + offset(w_ - 1 - x, y);
            for (std::size_t i = 0; i < stride; ++i) std::swap(a[i], b[i]);
        }
    }
}

void Image::flip_vertically() {
    const std::size_t row = w_ * channels_;
    for (int y = 0; y < h_ / 2; ++y) {
        std::uint8_t* a = data_.data() + offset(0, y);
        std::uint8_t* b = data_.data() + offset(0, h_ - 1 - y);
        for (std::size_t i = 0; i < row; ++i) std::swap(a[i], b[i]);
    }
}

Image Image::load_tga(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) return Image{};

    TgaHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!in.good()) return Image{};

    const int channels = header.bits_per_pixel >> 3;
    if (header.width <= 0 || header.height <= 0) return Image{};
    if (channels != Grayscale && channels != GrayAlpha && channels != RGB && channels != RGBA) return Image{};

    Image img(header.width, header.height, channels);
    const std::size_t nbytes = img.pixel_count() * channels;

    const bool is_rle = (header.image_type == kTypeRleTrueColor || header.image_type == kTypeRleGray);
    const bool is_raw = (header.image_type == kTypeTrueColor || header.image_type == kTypeGray);
    if (is_raw) {
        in.read(reinterpret_cast<char*>(img.data_.data()), nbytes);
        if (!in.good()) return Image{};
    } else if (is_rle) {
        if (!img.load_rle(in)) return Image{};
    } else {
        return Image{};
    }

    if (!(header.image_descriptor & kOriginTopLeft)) img.flip_vertically();
    if (header.image_descriptor & kOriginRight) img.flip_horizontally();
    return img;
}

bool Image::load_rle(std::istream& in) {
    const std::size_t pixels = pixel_count();
    const std::size_t stride = channels_;
    std::size_t written = 0;
    std::uint8_t chunk[4] = {0, 0, 0, 0};

    while (written < pixels) {
        const int header = in.get();
        if (!in.good()) return false;

        if (header < 128) {
            for (int i = 0; i <= header && written < pixels; ++i) {
                in.read(reinterpret_cast<char*>(chunk), stride);
                if (!in.good()) return false;
                std::memcpy(data_.data() + written * stride, chunk, stride);
                ++written;
            }
        } else {
            in.read(reinterpret_cast<char*>(chunk), stride);
            if (!in.good()) return false;
            for (int i = 0; i < header - 127 && written < pixels; ++i) {
                std::memcpy(data_.data() + written * stride, chunk, stride);
                ++written;
            }
        }
    }
    return true;
}

bool Image::save_tga(const std::string& filename, bool rle) const {
    if (!valid()) return false;

    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) return false;

    TgaHeader header = {};
    header.width = static_cast<std::uint16_t>(w_);
    header.height = static_cast<std::uint16_t>(h_);
    header.bits_per_pixel = static_cast<std::uint8_t>(channels_ << 3);
    header.image_descriptor = kOriginTopLeft;
    if (rle) {
        header.image_type = (channels_ == Grayscale) ? kTypeRleGray : kTypeRleTrueColor;
    } else {
        header.image_type = (channels_ == Grayscale) ? kTypeGray : kTypeTrueColor;
    }

    out.write(reinterpret_cast<const char*>(&header), sizeof(header));
    if (!out.good()) return false;

    if (rle) {
        if (!save_rle(out)) return false;
    } else {
        out.write(reinterpret_cast<const char*>(data_.data()),
                  data_.size());
        if (!out.good()) return false;
    }

    const std::uint8_t zeros[8] = {0};
    out.write(reinterpret_cast<const char*>(zeros), sizeof(zeros));
    out.write(reinterpret_cast<const char*>(kFooter), sizeof(kFooter));
    return out.good();
}

bool Image::save_rle(std::ostream& out) const {
    const std::uint8_t* base = data_.data();
    const std::size_t stride = channels_;

    for (int y = 0; y < h_; ++y) {
        int x = 0;
        while (x < w_) {
            const std::uint8_t* p = base + offset(x, y);

            int run = 1;
            while (x + run < w_ && run < kMaxChunk &&
                   std::memcmp(p, base + offset(x + run, y), stride) == 0) {
                ++run;
            }

            if (run >= 2) {
                out.put(static_cast<char>(run + 127));
                out.write(reinterpret_cast<const char*>(p), stride);
                x += run;
            } else {
                int count = 1;
                while (x + count < w_ && count < kMaxChunk) {
                    const std::uint8_t* a = base + offset(x + count - 1, y);
                    const std::uint8_t* b = base + offset(x + count, y);
                    if (std::memcmp(a, b, stride) == 0) break;
                    ++count;
                }
                out.put(static_cast<char>(count - 1));
                out.write(reinterpret_cast<const char*>(p), count * stride);
                x += count;
            }
            if (!out.good()) return false;
        }
    }
    return true;
}
