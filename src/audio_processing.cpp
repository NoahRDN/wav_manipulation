#include "audio_processing.hpp"
#include "binary_utils.hpp"

#include <stdexcept>
#include <cmath>
#include <limits>
#include <algorithm>
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

int32_t findMaxAmplitude16(
    const std::vector<int16_t>& samples
) {
    int32_t maxAmplitude = 0;

    for (int16_t sample : samples) {
        int32_t value = static_cast<int32_t>(sample);
        int32_t absValue = std::abs(value);

        if (absValue > maxAmplitude) {
            maxAmplitude = absValue;
        }
    }

    return maxAmplitude;
}

std::vector<int16_t> normalize16(
    const std::vector<int16_t>& samples,
    double targetRatio
) {
    std::vector<int16_t> result;
    result.reserve(samples.size());

    int32_t maxAmplitude = findMaxAmplitude16(samples);

    if (maxAmplitude == 0) {
        return samples;
    }

    const double maxAllowed =
        static_cast<double>(std::numeric_limits<int16_t>::max());

    const double targetAmplitude = maxAllowed * targetRatio;

    const double gain =
        targetAmplitude / static_cast<double>(maxAmplitude);

    for (int16_t sample : samples) {
        double normalized =
            static_cast<double>(sample) * gain;

        if (normalized > maxAllowed) {
            normalized = maxAllowed;
        }

        if (normalized < static_cast<double>(std::numeric_limits<int16_t>::min())) {
            normalized = static_cast<double>(std::numeric_limits<int16_t>::min());
        }

        result.push_back(
            static_cast<int16_t>(std::round(normalized))
        );
    }

    return result;
}

std::vector<int16_t> extractChannel16(
    const std::vector<int16_t>& samples,
    uint16_t numChannels,
    uint16_t channelIndex
) {
    if (numChannels == 0) {
        throw std::runtime_error("Nombre de canaux invalide");
    }

    if (channelIndex >= numChannels) {
        throw std::runtime_error("Index de canal invalide");
    }

    std::vector<int16_t> result;

    size_t frameCount = samples.size() / numChannels;
    result.reserve(frameCount);

    for (size_t frame = 0; frame < frameCount; frame++) {
        result.push_back(samples[frame * numChannels + channelIndex]);
    }

    return result;
}

std::vector<int16_t> stereoTo21(
    const std::vector<int16_t>& samples,
    uint16_t numChannels,
    bool lowPassSub,
    double alpha
) {
    if (numChannels != 2) {
        throw std::runtime_error("Le passage 2.0 vers 2.1 nécessite un fichier stereo");
    }

    if (alpha <= 0.0 || alpha > 1.0) {
        throw std::runtime_error("alpha doit etre dans l'intervalle ]0, 1]");
    }

    std::vector<int16_t> result;

    size_t frameCount = samples.size() / 2;
    result.reserve(frameCount * 3);

    double previousSub = 0.0;

    for (size_t frame = 0; frame < frameCount; frame++) {
        int16_t left  = samples[frame * 2];
        int16_t right = samples[frame * 2 + 1];

        int32_t subValue =
            (static_cast<int32_t>(left) + static_cast<int32_t>(right)) / 2;

        if (lowPassSub) {
            previousSub = alpha * subValue + (1.0 - alpha) * previousSub;
            subValue = static_cast<int32_t>(std::round(previousSub));
        }

        result.push_back(left);
        result.push_back(right);
        result.push_back(static_cast<int16_t>(subValue));
    }

    return result;
}

static int16_t clampToInt16(int32_t value) {
    if (value > 32767) {
        return 32767;
    }

    if (value < -32768) {
        return -32768;
    }

    return static_cast<int16_t>(value);
}

std::vector<int16_t> stereoTo51(
    const std::vector<int16_t>& samples,
    uint16_t numChannels
) {
    if (numChannels != 2) {
        throw std::runtime_error("Le passage en 5.1 nécessite un fichier stereo");
    }

    std::vector<int16_t> result;

    size_t frameCount = samples.size() / 2;
    result.reserve(frameCount * 6);

    for (size_t frame = 0; frame < frameCount; frame++) {
        int16_t left  = samples[frame * 2];
        int16_t right = samples[frame * 2 + 1];

        int16_t L = left;
        int16_t R = right;

        int16_t C = clampToInt16(
            static_cast<int32_t>(left) + static_cast<int32_t>(right)
        );

        int16_t LFE = clampToInt16(
            (static_cast<int32_t>(left) + static_cast<int32_t>(right)) / 2
        );

        int16_t Ls = static_cast<int16_t>(left / 2);
        int16_t Rs = static_cast<int16_t>(right / 2);

        result.push_back(L);
        result.push_back(R);
        result.push_back(C);
        result.push_back(LFE);
        result.push_back(Ls);
        result.push_back(Rs);
    }

    return result;
}