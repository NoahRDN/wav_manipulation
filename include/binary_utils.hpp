#ifndef BINARY_UTILS_HPP
#define BINARY_UTILS_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

uint16_t readUInt16LE(const std::vector<uint8_t>& buffer, size_t offset);
uint32_t readUInt32LE(const std::vector<uint8_t>& buffer, size_t offset);

void writeUInt16LE(std::vector<uint8_t>& buffer, size_t offset, uint16_t value);
void writeUInt32LE(std::vector<uint8_t>& buffer, size_t offset, uint32_t value);

#endif