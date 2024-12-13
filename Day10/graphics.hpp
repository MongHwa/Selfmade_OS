#include "frame_buffer_config.hpp"

template<typename T>
struct Vector2D {
    T x, y;

    template<typename U>
    Vector2D<T>& operator +=(const Vector2D<U>& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
};

template<typename T, typename U>
Rectangle<T> operator&(const Rectangle<T>& lhs, const Rectangle<U>& rhs) {
    const auto lhs_end = lhs.pos + lhs.size;
    const auto rhs_end = rhs.pos + rhs.size;

    if(lhs_end.x < rhs_pos.x || lhs_end.y < rhs_pos.y ||
       rhs_end.x < lhs_pos.x || rhs_end.y < lhs_pos.y) {
        return {{0, 0}, {0, 0}};
    }

    auto new_pos = ElementMax(lhs.pos, rhs.pos);
    auto new_size = ElementMin(lhs_end, rhs_end) - new_pos;
    return {new_pos, new_size};
}

struct PixelColor {
    uint8_t r, g, b;
};

class PixelWriter {
    public:
        PixelWriter(const FrameBufferConfig& config) : config_{config} {}
        virtual ~PixelWriter() = default;
        virtual void Write(int x, int y, const PixelColor& c) = 0;
    
    protected:
        uint8_t* PixelAt(int x, int y) {
            return config_.frame_buffer + 4 * (config_.pixels_per_scan_line*y + x);
        }
    
    private:
        const FrameBufferConfig& config_;
};

class RGBResv8BitPerColorPixelWriter : public PixelWriter {
    public:
        using PixelWriter::PixelWriter;
        virtual void Write(int x, int y, const PixelColor& c) override;
};

class BGRResv8BitPerColorPixelWriter : public PixelWriter {
    public:
        using PixelWriter::PixelWriter;
        virtual void Write(int x, int y, const PixelColor& c) override;
};

void DrawRectangle(PixelWriter& writer, const Vector2D<int>& pos,
                   const Vector2D<int>& size, const PixelColor& c);

void FillRectangle(PixelWriter& writer, const Vector2D<int>& pos,
                   const Vector2D<int>& size, const PixelColor& c);