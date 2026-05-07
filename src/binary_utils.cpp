#include "binary_utils.hpp"

uint16_t readUInt16LE(const std::vector<uint8_t>& buffer, size_t offset) {
    return buffer[offset] | (buffer[offset + 1] << 8);
}

uint32_t readUInt32LE(const std::vector<uint8_t>& buffer, size_t offset) {
    return buffer[offset]
         | (buffer[offset + 1] << 8)
         | (buffer[offset + 2] << 16)
         | (buffer[offset + 3] << 24);
}