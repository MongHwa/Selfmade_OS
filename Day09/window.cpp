#include "window.hpp"

Window::Window(int width, int height, PixelFormat shadow_format) : width_{width}, height_{height} {
    data_.resize(height);
    for(int y = 0; y < height; y++) {
        data_[y].resize(width);
    }

    FrameBufferConfig config{};
    config.frame_buffer = nullptr;
    config.horizontal_resolution = width;
    config.vertical_resolution = height;
    config.pixel_format = shadow_format;

    if(auto err = shadow_buffer_.Initialize(config)) {
        Log(kError, "failed to initialize shadow buffer: %s at %s:%d\n",
            err.Name(), err.File(), err.Line());
    }
}

void Window::DrawTo(FrameBuffer& dst, Vector2D<int> position) {
    if(!transparent_color_) {
        dst.Copy(position, shadow_buffer_);
        return;
    }

    const auto tc = transparent_color_.value();
    for(int y = 0; y < Height(); y++) {
        for(int x = 0; x < Width(); x++) {
            const auto c = At(x, y);
            if(c != tc) {
                writer.Write(position.x + Vector2D<int>(x, y), c);
            }
        }
    }
}

void SetTransparentColor(std::optional<PixelColor> c) {
    transparent_color_ = c;
}

const PixelColor& Window::At(Vector2D<int> pos) const {
    return data_[pos.y][pos.x];
}

void Window::Write(Vector2D<int> pos, PixelColor c) {
    data_[pos.y][pos.x] = c;
    shadow_buffer_.Writer().Write(pos, c);
}

void Window::Move(Vector2D<int> dst_pos, const Rectangle<int>& src) {
    shadow_buffer_.Move(dst_pos, src);
}