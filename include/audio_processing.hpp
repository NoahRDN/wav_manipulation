#ifndef AUDIO_PROCESSING_HPP
#define AUDIO_PROCESSING_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

std::vector<int16_t> extractSamples16Bits(
    const std::vector<uint8_t>& buffer,
    size_t audioOffset,
    uint32_t dataSize
);

std::vector<int16_t> downsampleBy2ByFrames(
    const std::vector<int16_t>& samples,
    uint16_t numChannels
);

std::vector<uint8_t> samples16ToBytes(
    const std::vector<int16_t>& samples
);

#endif