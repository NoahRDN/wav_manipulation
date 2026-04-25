#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

uint16_t readUInt16LE(const std::vector<uint8_t>& buffer, size_t offset) {
    return buffer[offset] | (buffer[offset + 1] << 8);
}

uint32_t readUInt32LE(const std::vector<uint8_t>& buffer, size_t offset) {
    return buffer[offset]
         | (buffer[offset + 1] << 8)
         | (buffer[offset + 2] << 16)
         | (buffer[offset + 3] << 24);
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

    return 0;
}
