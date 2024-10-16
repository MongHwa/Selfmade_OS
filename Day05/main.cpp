#include "frame_buffer_config.hpp";
#include "graphics.hpp";
#include "font.hpp";
#include <cstdio>

//displacement new 구현
void* operator new(size_t size, void* buf) {
    return buf;
}

void operator delete(void* obj) noexcept { 
}

//C언어 형식으로 함수 정의
//커널 Main
extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
    //displacement new 구현 참고
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

    int i = 0;
    for(char c = '!'; c <= '~'; c++, i++) {
        WriteAscii(*pixel_writer, 8*i, 50, c {0, 0, 0});
    }
    WriteString(*pixel_writer, 0, 66, "Hello, world!", {0, 0, 255});

    char buf[128];
    sprintf(buf, "1 + 2 = %d", 1+2);
    WriteString(*pixel_writer, 0, 82, buf, {0, 0, 0});

    while (1) __asm__("hlt");
}