#ifndef AUDIO_PROCESSING_HPP
#define AUDIO_PROCESSING_HPP

#include <cstdint>
#include <vector>

std::vector<int16_t> extractSamples16Bits(
    const std::vector<uint8_t>& buffer,
    size_t audioOffset,
    uint32_t dataSize
);

std::vector<int16_t> downsampleBy2(
    const std::vector<int16_t>& samples
);

#endif