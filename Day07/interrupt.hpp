#pragma once

#include <cstdint>

union InterruptDescriptorAttribute {
    uint16_t data;
    struct {
        uint16_t interrupt_stack_table : 3;
        uint16_t : 5;
        DescriptorType type : 4;
        uint16_t : 1;
        uint16_t descriptor_privilege_level : 2;
        uint16_t present : 1;
    } __attribute__((packed)) bits;
} __attribute__((packed)); //packed: 틈새 삽입 방지

struct InterruptDescriptor {
    uint16_t offset_low;
    uint16_t segment_selector;
    InterruptDescriptorAttribute attr;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

void NotifyEndOfInterrupt();
void SetIDTEntry(InterruptDescriptor& desc,
                 InterruptDescriptorAttribue attr,
                 uint64_t offset,
                 uint16_t segment_selector);