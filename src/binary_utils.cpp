#include "../include/binary_utils.hpp"

#include <stdexcept>

uint16_t readUInt16LE(const std::vector<uint8_t>& buffer, size_t offset) {
    if (offset + 1 >= buffer.size()) {
        throw std::out_of_range("readUInt16LE : offset invalide");
    }

    return static_cast<uint16_t>(
        static_cast<uint16_t>(buffer[offset]) |
        static_cast<uint16_t>(buffer[offset + 1] << 8)
    );
}

uint32_t readUInt32LE(const std::vector<uint8_t>& buffer, size_t offset) {
    if (offset + 3 >= buffer.size()) {
        throw std::out_of_range("readUInt32LE : offset invalide");
    }

    return static_cast<uint32_t>(buffer[offset])
         | (static_cast<uint32_t>(buffer[offset + 1]) << 8)
         | (static_cast<uint32_t>(buffer[offset + 2]) << 16)
         | (static_cast<uint32_t>(buffer[offset + 3]) << 24);
}

void writeUInt16LE(std::vector<uint8_t>& buffer, size_t offset, uint16_t value) {
    if (offset + 1 >= buffer.size()) {
        throw std::out_of_range("writeUInt16LE : offset invalide");
    }

    buffer[offset]     = static_cast<uint8_t>(value & 0xFF);
    buffer[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

void writeUInt32LE(std::vector<uint8_t>& buffer, size_t offset, uint32_t value) {
    if (offset + 3 >= buffer.size()) {
        throw std::out_of_range("writeUInt32LE : offset invalide");
    }

    buffer[offset]     = static_cast<uint8_t>(value & 0xFF);
    buffer[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    buffer[offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
    buffer[offset + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
}