#include "audio_processing.hpp"
#include "binary_utils.hpp"

std::vector<int16_t> extractSamples16Bits(
    const std::vector<uint8_t>& buffer,
    size_t audioOffset,
    uint32_t dataSize
) {
    std::vector<int16_t> samples;

    for (size_t i = audioOffset; i < audioOffset + dataSize; i += 2) {

        uint16_t sampleUInt =
            buffer[i] |
            (buffer[i + 1] << 8);

        int16_t sample = static_cast<int16_t>(sampleUInt);

        samples.push_back(sample);
    }

    return samples;
}

std::vector<int16_t> downsampleBy2(
    const std::vector<int16_t>& samples
) {
    std::vector<int16_t> newSamples;

    for (size_t i = 0; i + 1 < samples.size(); i += 2) {

        int32_t average =
            (samples[i] + samples[i + 1]) / 2;

        newSamples.push_back(
            static_cast<int16_t>(average)
        );
    }

    return newSamples;
}