#include "wav_file.hpp"

#include <iostream>

int main() {
    try {
        std::vector<uint8_t> buffer = readBinaryFile("input.wav");

        std::cout << "Fichier charge : " << buffer.size() << " octets\n";

        WavInfo info = parseWavInfo(buffer);

        std::cout << "Canaux : " << info.numChannels << "\n";
        std::cout << "Frequence : " << info.sampleRate << " Hz\n";
        std::cout << "Bits par echantillon : " << info.bitsPerSample << "\n";
        std::cout << "Offset chunk data : " << info.dataChunkOffset << "\n";
        std::cout << "Taille data : " << info.dataSize << " octets\n";
        std::cout << "Debut donnees audio : " << info.audioDataOffset << "\n";
    }
    catch (const std::exception& error) {
        std::cerr << "Erreur : " << error.what() << "\n";
        return 1;
    }

    return 0;
}