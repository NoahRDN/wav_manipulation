#include "wav_file.hpp"
#include "binary_utils.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

std::vector<uint8_t> readBinaryFile(const std::string& path) {
    std::ifstream file{path, std::ios::binary};

    if (!file) {
        throw std::runtime_error("Impossible d'ouvrir le fichier : " + path);
    }

    return std::vector<uint8_t>{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };
}

bool isWavFile(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 12) {
        return false;
    }

    bool isRiff =
        buffer[0] == 'R' &&
        buffer[1] == 'I' &&
        buffer[2] == 'F' &&
        buffer[3] == 'F';

    bool isWave =
        buffer[8] == 'W' &&
        buffer[9] == 'A' &&
        buffer[10] == 'V' &&
        buffer[11] == 'E';

    return isRiff && isWave;
}

size_t findDataChunk(const std::vector<uint8_t>& buffer) {
    for (size_t i = 0; i + 8 < buffer.size(); i++) {
        if (buffer[i] == 'd' &&
            buffer[i + 1] == 'a' &&
            buffer[i + 2] == 't' &&
            buffer[i + 3] == 'a') {
            return i;
        }
    }

    return std::string::npos;
}

WavInfo parseWavInfo(const std::vector<uint8_t>& buffer) {
    if (!isWavFile(buffer)) {
        throw std::runtime_error("Le fichier n'est pas un WAV RIFF valide");
    }

    WavInfo info{};

    info.numChannels = readUInt16LE(buffer, 22);
    info.sampleRate = readUInt32LE(buffer, 24);
    info.bitsPerSample = readUInt16LE(buffer, 34);

    info.dataChunkOffset = findDataChunk(buffer);

    if (info.dataChunkOffset == std::string::npos) {
        throw std::runtime_error("Chunk data introuvable");
    }

    info.dataSize = readUInt32LE(buffer, info.dataChunkOffset + 4);
    info.audioDataOffset = info.dataChunkOffset + 8;

    return info;
}