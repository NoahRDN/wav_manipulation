#ifndef WAV_FILE_HPP
#define WAV_FILE_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct WavInfo {
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    size_t dataChunkOffset;
    uint32_t dataSize;
    size_t audioDataOffset;
};

std::vector<uint8_t> readBinaryFile(const std::string& path);
void writeBinaryFile(const std::string& path, const std::vector<uint8_t>& buffer);

bool isWavFile(const std::vector<uint8_t>& buffer);
size_t findDataChunk(const std::vector<uint8_t>& buffer);
WavInfo parseWavInfo(const std::vector<uint8_t>& buffer);

void updateHeaderAfterDownsamplingBy2(
    std::vector<uint8_t>& buffer,
    const WavInfo& info,
    uint32_t newDataSize
);

void updateHeaderAfterQuantization8Bits(
    std::vector<uint8_t>& buffer,
    const WavInfo& info,
    uint32_t newDataSize
);

void updateHeaderAfterMonoExtraction(
    std::vector<uint8_t>& buffer,
    const WavInfo& info,
    uint32_t newDataSize
);

void updateHeaderAfterStereoTo21(
    std::vector<uint8_t>& buffer,
    const WavInfo& info,
    uint32_t newDataSize
);

#endif