#include <cstdint>
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"

int WritePixel(const FrameBufferConfig& config,
               int x, int y, const PixelColor& c) {
    
    const int pixel_position = config.pixels_per_scan_line*y + x;
    if(config.pixel_format == kPixelRGBResv8BitPerColor) {
        uint8_t* p = &config.frame_buffer[4 * pixel_position];
        p[0] = c.r;
        p[1] = c.g;
        p[2] = c.b;
    } else if(config.pixel_format == kPixelBGRResv8BitPerColor) {
        uint8_t* p = &config.frame_buffer[4 * pixel_position];
        p[0] = c.b;
        p[1] = c.g;
        p[2] = c.r;
    } else {
        return -1;
    }

    return 0;
}

//for pixel_writer pointer
//start of code
char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;
//end of code


//for displacement new
//start of code
void* operator new(size_t size, void* buf) {
    return buf;
}

void operator delete(void* obj) noexcept {
}
//end of code


//for printk() function
//start of code
int printk(const char* format, ...) {
    va_list ap;
    int result;
    char s[1024];

    va_start(ap, format);
    result = vsprintf(s, format, ap);
    va_end(ap);

    console->PutString(s);
    return result;
}
//end of code


extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {

    switch(frame_buffer_config.pixel_format) {
        case kPixelRGBResv8BitPerColor:
            pixel_writer = new(pixel_writer_buf)
                RGBResv8BitPerColorPixelWriter{frame_buffer_config};
            break;
        case kPixelBGRResv8BitPerColor:
            pixel_writer = new(pixel_writer_buf)
                BGRResv8BitPerColorPixelWriter{frame_buffer_config};
            break;
    }

    for(int x = 0; x < frame_buffer_config.horizontal_resolution; x++) {
        for(int y = 0; y < frame_buffer_config.vertical_resolution; y++) {
            pixel_writer->Write(x, y, {255, 255, 255});
        }
    }

    for(int x = 0; x < 200; x++) {
        for(int y = 0; y < 100; y++) {
            pixel_writer->Write(100+x, 100+y, {0, 255, 0});
        }
    }
    
    while(1) __asm__("hlt");
}