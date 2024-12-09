#include "segment.hpp"

namespace {
    std::array<SegmentDescriptor, 3> gdt;
}

//GDT 재구축
//1. 코드 세그먼트 (디스크립터 1)
void SetCodeSegment(SegmentDescriptor& desc,
                    DescriptorType type,
                    unsigned int descriptor_privilage_level,
                    uint32_t base,
                    uint32_t limit) {
    
    desc.data = 0;

    desc.bits.base_low = base & 0xffffu;
    desc.bits.base_middle = (base>>16) & 0xffu;
    desc.bits.base_high = (base>>24) & 0xffu;

    desc.bits.limit_low = limit & 0xffffu;
    desc.bits.limit_high = (limit>>16) & 0xfu;

    desc.bits.type = type;
    desc.bits.system_segment = 1;
    desc.bits.descriptor_privilage_level = descriptor_privilage_level;
    desc.bits.present = 1;
    desc.bits.available = 0;
    desc.bits.long_mode = 1;
    desc.bits.default_operation_size = 0; //long_mode가 1이면 반드시 0이어야 함.
    desc.bits.granularity = 1;
}

//2. 데이터 세그먼트 (디스크립터 2)
void SetDataSegment(SegmentDescriptor& desc,
                    DescriptorType type,
                    unsigned int descriptor_privileage_level,
                    uint32_t base,
                    uint32_t limit) {
    
    SetCodeSegment(desc, type, descriptor_privileage_level, base, limit);
    desc.bits.long_mode = 0;
    desc.bits.default_operation_size = 1;
}

//3. 세그먼트 구축 (디스크립터 0, 1, 2 결합 후 로드)
void SetupSegments() {
    gdt[0].data = 0;
    SetCodeSegment(gdt[1], DescriptorType::kExecuted, 0, 0, 0xfffff);
    SetDataSegment(gdt[2], DescriptorType::kReadWrite, 0, 0, 0xfffff);
    LoadGDT(sizeof(gdt)-1, reinterpret_cast<uintptr_t>(&gdt[0]));
}