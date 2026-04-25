#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>

uint16_t readUInt16LE(const std::vector<uint8_t>& buffer, size_t offset) {
    return buffer[offset] | (buffer[offset + 1] << 8);
}

uint32_t readUInt32LE(const std::vector<uint8_t>& buffer, size_t offset) {
    return buffer[offset]
         | (buffer[offset + 1] << 8)
         | (buffer[offset + 2] << 16)
         | (buffer[offset + 3] << 24);
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

int main() {
    std::ifstream file{"input.wav", std::ios::binary};

    if (!file) {
        std::cerr << "Erreur : impossible d'ouvrir input.wav\n";
        return 1;
    }

    std::vector<uint8_t> buffer{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    std::cout << "Fichier charge : " << buffer.size() << " octets\n";


    for (int i = 22; i < 24; i++) {
        printf("0x%02X ", buffer[i]);
    }
    
    printf("\n");

    for (int i = 22; i < 24; i++) {
        std::cout << (char)buffer[i];
    }
    
    std::cout << "\n";
    printf("%ld\n", buffer.size());
    
    // -----------------------------------------------------

    if (buffer[0] != 'R' || buffer[1] != 'I' || buffer[2] != 'F' || buffer[3] != 'F') {
        std::cerr << "Pas un fichier RIFF\n";
        return 1;
    }

    if (buffer[8] != 'W' || buffer[9] != 'A' || buffer[10] != 'V' || buffer[11] != 'E') {
        std::cerr << "Pas un fichier WAVE\n";
        return 1;
    }

    // -----------------------------

    uint16_t numChannels   = readUInt16LE(buffer, 22);
    uint32_t sampleRate    = readUInt32LE(buffer, 24);
    uint16_t bitsPerSample = readUInt16LE(buffer, 34);

    std::cout << "Canaux : " << numChannels << "\n";
    std::cout << "Frequence : " << sampleRate << " Hz\n";
    std::cout << "Bits par echantillon : " << bitsPerSample << "\n";

    // -----------------------------

    size_t dataChunkOffset = findDataChunk(buffer);

    if (dataChunkOffset == std::string::npos) {
        std::cerr << "Chunk data introuvable\n";
        return 1;
    }

    uint32_t dataSize = readUInt32LE(buffer, dataChunkOffset + 4);
    size_t audioDataOffset = dataChunkOffset + 8;

    std::cout << "Offset chunk data : " << dataChunkOffset << "\n";
    std::cout << "Taille data : " << dataSize << " octets\n";
    std::cout << "Debut donnees audio : " << audioDataOffset << "\n";

    return 0;
}
