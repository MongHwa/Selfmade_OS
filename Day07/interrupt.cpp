#include "interrupt.hpp"

void NotifyEndOfInterrupt() {
    //volatile : 최적화 X
    volatile auto end_of_interrupt = reinterpret_cast<uint32_t*>(0xfee000b0);
    *end_of_interrupt = 0;
}

//Interrupt Descriptor 구조체의 값 설정 함수
void SetIDTEntry(InterruptDescriptor& desc,
                 InterruptDescriptorAttribute attr,
                 uint64_t offset,
                 uint16_t segment_selector) {
    
    desc.attr = attr;
    desc.offset_low = offset & 0xffffu;
    desc.offset_middle = (offset>>16) & 0xffffu;
    desc.offset_high = (offset>>32);
    desc.segment_selecotr = segment_selector;
}