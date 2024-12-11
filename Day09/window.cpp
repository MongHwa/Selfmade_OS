#include "window.hpp"

Window::Window(int width, int height, PixelFormat shadow_format) : width_{width}, height_{height} {
    data_.resize(height);
    for(int y = 0; y < height; y++) {
        data_[y].resize(width);
    }
}

void Window::DrawTo(PixelWriter& writer, Vector2D<int> position) {
    if(!transparent_color_) {
        for(int y = 0; y < Height(); y++) {
            for(int x = 0; x < Width(); x++) {
                writer.Write(position.x + x, position.y + y, At(x, y));
            }
        }

        return;
    }

    const auto tc = transparent_color_.value();
    for(int y = 0; y < Height(); y++) {
        for(int x = 0; x < Width(); x++) {
            const auto c = At(x, y);
            if(c != tc) {
                writer.Write(position.x + x, position.y + y, c);
            }
        }
    }
}

void SetTransparentColor(std::optional<PixelColor> c) {
    transparent_color_ = c;
}