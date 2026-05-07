#ifndef WAV_FILE_HPP
#define WAV_FILE_HPP

#include <cstdint>
#include <string>
#include <vector>

struct WavInfo {
    uint16_t numChannels;
    uint32_t sampleRate;
    uint16_t bitsPerSample;

    size_t dataChunkOffset;
    uint32_t dataSize;
    size_t audioDataOffset;
};

std::vector<uint8_t> readBinaryFile(const std::string& path);
bool isWavFile(const std::vector<uint8_t>& buffer);
size_t findDataChunk(const std::vector<uint8_t>& buffer);
WavInfo parseWavInfo(const std::vector<uint8_t>& buffer);

#endif