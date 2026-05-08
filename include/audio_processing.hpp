#ifndef AUDIO_PROCESSING_HPP
#define AUDIO_PROCESSING_HPP

#include <cstddef>
#include <cstdint>
#include <vector>
#include <cstddef>

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

std::vector<uint8_t> quantize16To8(
    const std::vector<int16_t>& samples
);

size_t countSaturatedSamples(
    const std::vector<int16_t>& samples
);

std::vector<int16_t> softLimit16(
    const std::vector<int16_t>& samples,
    double thresholdRatio = 0.95
);

int32_t findMaxAmplitude16(
    const std::vector<int16_t>& samples
);

std::vector<int16_t> normalize16(
    const std::vector<int16_t>& samples,
    double targetRatio = 0.95
);

std::vector<int16_t> extractChannel16(
    const std::vector<int16_t>& samples,
    uint16_t numChannels,
    uint16_t channelIndex
);

#endif