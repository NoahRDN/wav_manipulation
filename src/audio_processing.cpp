#include "audio_processing.hpp"
#include "binary_utils.hpp"

#include <stdexcept>
#include <cmath>
#include <limits>

std::vector<int16_t> extractSamples16Bits(
    const std::vector<uint8_t>& buffer,
    size_t audioOffset,
    uint32_t dataSize
) {
    std::vector<int16_t> samples;

    size_t end = audioOffset + dataSize;

    if (end > buffer.size()) {
        throw std::runtime_error("Les donnees audio depassent la taille du fichier");
    }

    for (size_t i = audioOffset; i + 1 < end; i += 2) {
        uint16_t raw = readUInt16LE(buffer, i);
        int16_t sample = static_cast<int16_t>(raw);

        samples.push_back(sample);
    }

    return samples;
}

std::vector<int16_t> downsampleBy2ByFrames(
    const std::vector<int16_t>& samples,
    uint16_t numChannels
) {
    std::vector<int16_t> result;

    if (numChannels == 0) {
        return result;
    }

    size_t frameCount = samples.size() / numChannels;

    result.reserve((frameCount / 2) * numChannels);

    for (size_t frame = 0; frame + 1 < frameCount; frame += 2) {
        for (uint16_t ch = 0; ch < numChannels; ch++) {
            int16_t a = samples[frame * numChannels + ch];
            int16_t b = samples[(frame + 1) * numChannels + ch];

            int32_t average =
                (static_cast<int32_t>(a) + static_cast<int32_t>(b)) / 2;

            result.push_back(static_cast<int16_t>(average));
        }
    }

    return result;
}

std::vector<uint8_t> samples16ToBytes(
    const std::vector<int16_t>& samples
) {
    std::vector<uint8_t> bytes;

    bytes.reserve(samples.size() * 2);

    for (int16_t sample : samples) {
        uint16_t raw = static_cast<uint16_t>(sample);

        bytes.push_back(static_cast<uint8_t>(raw & 0xFF));
        bytes.push_back(static_cast<uint8_t>((raw >> 8) & 0xFF));
    }

    return bytes;
}

std::vector<uint8_t> quantize16To8(
    const std::vector<int16_t>& samples
) {
    std::vector<uint8_t> result;
    result.reserve(samples.size());

    for (int16_t sample16 : samples) {
        uint8_t sample8 = static_cast<uint8_t>(
            (static_cast<int32_t>(sample16) + 32768) / 256
        );

        result.push_back(sample8);
    }

    return result;
}

size_t countSaturatedSamples(
    const std::vector<int16_t>& samples
) {
    size_t count = 0;

    for (int16_t sample : samples) {
        if (sample == std::numeric_limits<int16_t>::max() ||
            sample == std::numeric_limits<int16_t>::min()) {
            count++;
        }
    }

    return count;
}

std::vector<int16_t> softLimit16(
    const std::vector<int16_t>& samples,
    double thresholdRatio
) {
    std::vector<int16_t> result;
    result.reserve(samples.size());

    const double maxAmp = static_cast<double>(
        std::numeric_limits<int16_t>::max()
    );

    const double threshold = maxAmp * thresholdRatio;
    const double range = maxAmp - threshold;

    for (int16_t sample : samples) {
        double x = static_cast<double>(sample);
        double sign = x < 0 ? -1.0 : 1.0;
        double absValue = std::abs(x);

        if (absValue <= threshold) {
            result.push_back(sample);
            continue;
        }

        double excess = absValue - threshold;

        double compressed =
            range * std::tanh(excess / range);

        double limited =
            threshold + compressed;

        if (limited > maxAmp) {
            limited = maxAmp;
        }

        int16_t newSample = static_cast<int16_t>(
            std::round(sign * limited)
        );

        result.push_back(newSample);
    }

    return result;
}