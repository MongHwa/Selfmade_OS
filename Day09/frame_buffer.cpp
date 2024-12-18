#include "frame_buffer.hpp"

//For Utility
namespace {
    int BytesPerPixel(PixelFormat format) {
        switch(format) {
            case kPixelRGBResv8BitPerColor: return 4;
            case kPixelBGRResv8BitPerColor: return 4;
        }

        return -1;
    }

    uint8_t* FrameAddrAt(Vector2D<int> pos, const FrameBufferConfig& config) {
        return config.frame_buffer + BytesPerPixel(config.pixel_format) *
            (config.pixels_per_scan_line * pox.y + pos.x);
    }

    int BytesPerScanLine(const FrameBufferConfig& config) {
        return BytesPerPixel(config.pixel_format) * config.pixels_per_scan_line;
    }

    Vector2D<int> FrameBufferSize(const FrameBufferConfig& config) {
        return {static_cast<int>(config.horizontal_resolution),
                static_cast<int>(config.vertical_resolution)};
    }
}

Error FrameBuffer::Initialize(const FrameBufferConfig& config) {
    config_ = config;

    const auto bits_per_pixel = BitsPerPixel(config_.pixel_format);
    if(bits_per_pixel <= 0) {
        return MAKE_ERROR(Error:kUnknownPixelFormat);
    }

    if(config_.frame_buffer) {
        buffer_.resize(0);
    } else {
        buffer_.resize(
            ((bits_per_pixel_+7) / 8) * config_.horizontal_resolution * config_.vertical_resolution);
        config_.frame_buffer = buffer_.data();
        config_.pixels_per_scan_line = config_.horizontal_resolution;
    }

    switch(config_.pixel_format) {
        case kPixelRGBResv8BitPerColor:
            writer_ = std::make_unique<RGBResv8BitPerColorPixelWriter>(config_);
            break;
        case kPixelBGRResv8BitPerColor:
            writer_ = std::make_unique<BGRResv8BitPerColorPixelWriter>(config_);
            break;
        default:
            return MAKE_ERROR(Error::kUnknownPixelFormat);
    }

    return MAKE_ERROR(Error::kSuccess);
}

int BitsPerPixel(PixelFormat format) {
    switch(format) {
        case kPixelRGBResv8BitPerColor: return 32;
        case kPixelBGRResv8BitPerColor: return 32;
    }

    return -1;
}

Error FrameBuffer::Copy(Vector2D<int> pos, const FrameBuffer& src) {
    if(config_.pixel_format != src.config_.pixel_format) {
        return MAKE_ERROR(Error::kUnknownPixelFormat);
    }

    const auto bits_per_pixel = BitsPerPixel(config_.pixel_format);
    if(bits_per_pixel <= 0) {
        return MAKE_ERROR(Error::kUnknownPixelFormat);
    }

    const auto dst_width = config_.horizontal_resolution;
    const auto dst_height = config_.vertical_resolution;
    const auto src_width = src.config_.horizontal_resolution;
    const auto src_height = src.config_.vertical_resolution;

    const int copy_start_dst_x = std::max(pos.x, 0);
    const int copy_start_dst_y = std::max(pos.y, 0);
    const int copy_end_dst_x = std::min(pos.x + src_width, dst_width);
    const int copy_end_dst_y = std::min(pos.y + src_height, dst_height);

    const auto bytes_per_pixel = (bits_per_pixel + 7) / 8;
    const auto bytes_per_copy_line = bytes_per_pixel * (copy_end_dst_x - copy_start_dst_x);

    uint8_t* dst_buf = config_.frame_buffer + bytes_per_pixel *
        (config_.pixels_per_scan_line * copy_start_dst_y + copy_start_dst_x);
    const uint8_t* src_buf = src.config_.frame_buffer;

    //복사 시작
    for(int dy = 0; dy < copy_end_dst_y - copy_start_dst_y; dy++) {
        memcpy(dst_buf, src_buf, bytes_per_copy_line);
        dst_buf += bytes_per_pixel * config_.pixels_per_scan_line;
        src_buf += bytes_per_pixel * src.config_.pixels_per_scan_line;
    }

    return MAKE_ERROR(Error::kSuccess);
}

void FrameBuffer::Move(Vector2D<int> dst_pos, const Rectangle<int>& src) {
    const auto bytes_per_pixel = BytesPerPixel(config_.pixel_format);
    const auto bytes_per_scan_line = BytesPerScanLine(config_);

    if(dst_pox.y < src_pos.Y) { //위로 이동
        uint8_t* dst_buf = FrameAddrAt{dst_pos, config_};
        const uint8_t* src_buf = FrameAddrAt(src.pos, config_);
        for(int y = 0; y < src.size.y; y++) {
            memcpy(dst_buf, src_buf, bytes_per_pixel * src.size.x);
            dst_buf += bytes_per_scan_line;
            src_buf += bytes_per_scan_line;
        }
    } else { //아래로 이동
        uint8_t* dst_buf = FrameAddrAt(dst_pos + Vector2D<int>{0, src.size.y-1}, config_);
        const uint8_t* src_buf = FrameAddrAt(src.pos + Vector2D<int>{0, src.size.y-1}, config_);
        for(int y = 0; y < src.size.y; y++) {
            memcpy(dst_buf, src_buf, bytes_per_pixel * src.size.x);
            dst_buf -= bytes_per_scan_line;
            src_buf -= bytes_per_scan_line;
        }
    }
}