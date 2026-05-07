#include "wav_file.hpp"
#include "audio_processing.hpp"

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

        std::vector<int16_t> samples =
            extractSamples16Bits(
                buffer,
                info.audioDataOffset,
                info.dataSize
            );

        std::cout << "Nombre de samples : "
                << samples.size()
                << "\n";

        std::vector<int16_t> downsampled =
            downsampleBy2(samples);

        std::cout << "Samples apres division par 2 : "
                << downsampled.size()
                << "\n";

        std::cout << "Nouvelle frequence : "
                << info.sampleRate / 2
                << " Hz\n";
    }
    catch (const std::exception& error) {
        std::cerr << "Erreur : " << error.what() << "\n";
        return 1;
    }

    return 0;
}