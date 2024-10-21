#pragma once

#include <cstdint>
#include <array>
//#include "error.hpp"

const uint16_t kConfigAddress = 0x0cf8;
const uint16_t kConfigData = 0x0cfc;

struct Device {
    uint8_t bus, device, function, header_type;
};

void WriteAddress(uint32_t address);
void WriteData(uint32_t value);
uint32_t ReadData();

uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function);
uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function);
uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function);
uint32_t ReadClassCode(uint8_t bus, uint8_t device, uint8_t function);

Error ScanAllBus();