#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "console.hpp"
#include <cstdio>

//displacement new 구현
void* operator new(size_t size, void* buf) {
    return buf;
}

void operator delete(void* obj) noexcept { 
}

//global area
char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;

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

    Console console{*pixel_writer, {0, 0, 0}, {255, 255, 255}};

    //줄바꿈 테스트
    char buf[128];
    for(int i = 0; i < 27; i++) {
        sprintf(buf, "line %d\n", i);
        console.PutString(buf);
    }

    while (1) __asm__("hlt");
}