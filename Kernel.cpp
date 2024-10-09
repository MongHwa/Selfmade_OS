//uintX_t 자료형 사용 위해 include
#include <cstdint>

//C언어 형식으로 함수 정의
extern "C" void KernelMain(uint64_t frame_buffer_base,
                           uint64_t frame_buffer_size) {
    
    uint8_t* frame_buffer = reinterpret_cast<uint8_t*>(frame_buffer_base);
    for(uint64_t i = 0; i < frame_buffer_size; i++) {
        frame_buffer[i] = i % 256;
    }

    while (1) __asm__("hlt");
}