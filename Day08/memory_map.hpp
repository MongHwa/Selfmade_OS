//MikanLoaderPkg/main.c에서도, kernel/main.cpp에서도 같은 구조체 정의를 사용할 필요 있음
#pragma once

#include <stdint.h>

struct MemoryMap {
    unsigned long long buffer_size;
    void* buffer;
    unsigned long long map_size;
    unsigned long long map_key;
    unsigned long long descriptor_size;
    uint32_t descriptor_version;
};

struct MemoryDescriptor {
    uint32_t type;
    uintptr_t physical_start;
    uintptr_t virtual_start;
    uint64_t number_of_pages;
    uint64_t attribute;
};

//main.c에서는 사용될 수 없도록 함
#ifdef __cplusplus
enum class MemoryType {
    kEfiReservedMemoryType,
    kEfiLoaderCode,
    kEfiLoaderData,
    kEfiBootServicesCode,
    kEfiBootServicesData,
    kEfiRuntimeServicesCode,
    kEfiRuntimeServicesData,
    kEfiConventionalMemory,
    kEfiUnusableMemory,
    kEfiACPIReclaimMemory,


};

inline bool operator== (uint32_t lhs, MemoryType rhs) {
    return lhs == static_cast<uint32_t>(rhs);
}

inline bool operator== (MemoryType lhs, uint32_t rhs) {
    return rhs == lhs;
}

//빈 영역 판정
inline bool IsAvailabe(MemoryType memory_type) {
    //UEFI를 벗어난 후(ExitBootServices()가 호출된 직후) 빈 영역으로 다뤄도 좋은 메모리 타입들
    return
        memory_type == MemoryType::kEfiBootServicesCode ||
        memory_type == MemoryType::kEfiBootServicesData ||
        memory_type == MemoryType::kEfiConventionalMemory;
}

const int kUEFIPageSize = 4096;
#endif