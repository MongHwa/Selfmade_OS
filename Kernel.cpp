//uintX_t 자료형 사용 위해 include
#include <cstdint>

struct PixelColor {
    uint8_t r, g, b;
};

//한 개의 점을 그리는 함수
//retval 0 -> 성공 / -1 -> 실패
int WritePixel(const FrameBufferConfig& config,
               int x, int y, const PixelColor& c) {
    //가로축: x / 세로축: y로 설정
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


//C언어 형식으로 함수 정의
//커널 Main
extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
    
    for(int x = 0; x < frame_buffer_config.horizontal_resolution; x++) {
        for(int y = 0; y < frame_buffer_config.vertical_resolution; y++) {
            WritePixel(frame_buffer_config, x, y, {255, 255, 255});
        }
    }

    for(int x = 0; x < 200; x++) {
        for(int y = 0; y < 100; y++) {
            WritePixel(frame_buffer_config, 100+x, 100+y, {0, 255, 0});
        }
    }

    while (1) __asm__("hlt");
}