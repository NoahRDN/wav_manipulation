#include "wav_file.hpp"
#include "binary_utils.hpp"

#include <fstream>
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

void writeBinaryFile(const std::string& path, const std::vector<uint8_t>& buffer) {
    std::ofstream file{path, std::ios::binary};

    if (!file) {
        throw std::runtime_error("Impossible d'ecrire le fichier : " + path);
    }

    file.write(
        reinterpret_cast<const char*>(buffer.data()),
        static_cast<std::streamsize>(buffer.size())
    );
}

bool isWavFile(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 44) {
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
    size_t offset = 12;

    while (offset + 8 <= buffer.size()) {
        bool isData =
            buffer[offset] == 'd' &&
            buffer[offset + 1] == 'a' &&
            buffer[offset + 2] == 't' &&
            buffer[offset + 3] == 'a';

        uint32_t chunkSize = readUInt32LE(buffer, offset + 4);

        if (isData) {
            return offset;
        }

        offset += 8 + chunkSize;

        if (chunkSize % 2 != 0) {
            offset += 1;
        }
    }

    return std::string::npos;
}

WavInfo parseWavInfo(const std::vector<uint8_t>& buffer) {
    if (!isWavFile(buffer)) {
        throw std::runtime_error("Le fichier n'est pas un WAV RIFF valide");
    }

    WavInfo info{};

    info.audioFormat   = readUInt16LE(buffer, 20);
    info.numChannels   = readUInt16LE(buffer, 22);
    info.sampleRate    = readUInt32LE(buffer, 24);
    info.byteRate      = readUInt32LE(buffer, 28);
    info.blockAlign    = readUInt16LE(buffer, 32);
    info.bitsPerSample = readUInt16LE(buffer, 34);

    info.dataChunkOffset = findDataChunk(buffer);

    if (info.dataChunkOffset == std::string::npos) {
        throw std::runtime_error("Chunk data introuvable");
    }

    info.dataSize = readUInt32LE(buffer, info.dataChunkOffset + 4);
    info.audioDataOffset = info.dataChunkOffset + 8;

    return info;
}

void updateHeaderAfterDownsamplingBy2(
    std::vector<uint8_t>& buffer,
    const WavInfo& info,
    uint32_t newDataSize
) {
    uint32_t newSampleRate = info.sampleRate / 2;
    uint32_t newByteRate = newSampleRate * info.blockAlign;
    uint32_t newChunkSize = static_cast<uint32_t>(buffer.size() - 8);

    writeUInt32LE(buffer, 4, newChunkSize);
    writeUInt32LE(buffer, 24, newSampleRate);
    writeUInt32LE(buffer, 28, newByteRate);
    writeUInt32LE(buffer, info.dataChunkOffset + 4, newDataSize);
}

void updateHeaderAfterQuantization8Bits(
    std::vector<uint8_t>& buffer,
    const WavInfo& info,
    uint32_t newDataSize
) {
    uint16_t newBitsPerSample = 8;
    uint16_t newBlockAlign =
        info.numChannels * (newBitsPerSample / 8);

    uint32_t newByteRate =
        info.sampleRate * newBlockAlign;

    uint32_t newChunkSize =
        static_cast<uint32_t>(buffer.size() - 8);

    writeUInt32LE(buffer, 4, newChunkSize);
    writeUInt32LE(buffer, 28, newByteRate);
    writeUInt16LE(buffer, 32, newBlockAlign);
    writeUInt16LE(buffer, 34, newBitsPerSample);
    writeUInt32LE(buffer, info.dataChunkOffset + 4, newDataSize);
}

void updateHeaderAfterMonoExtraction(
    std::vector<uint8_t>& buffer,
    const WavInfo& info,
    uint32_t newDataSize
) {
    uint16_t newNumChannels = 1;

    uint16_t newBlockAlign =
        newNumChannels * (info.bitsPerSample / 8);

    uint32_t newByteRate =
        info.sampleRate * newBlockAlign;

    uint32_t newChunkSize =
        static_cast<uint32_t>(buffer.size() - 8);

    writeUInt32LE(buffer, 4, newChunkSize);
    writeUInt16LE(buffer, 22, newNumChannels);
    writeUInt32LE(buffer, 28, newByteRate);
    writeUInt16LE(buffer, 32, newBlockAlign);
    writeUInt32LE(buffer, info.dataChunkOffset + 4, newDataSize);
}