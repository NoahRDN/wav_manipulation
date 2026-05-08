#include "wav_file.hpp"
#include "audio_processing.hpp"
#include "binary_utils.hpp"


#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    try {
        std::vector<uint8_t> buffer = readBinaryFile("input.wav");

        std::cout << "Fichier charge : " << buffer.size() << " octets\n";
        std::cout << "Verification format WAV... " << buffer[0] << buffer[1] << buffer[2] << buffer[3] << "\n";

        std::cout << "Taille du fichier : " << readUInt32LE(buffer, 4) << " octets\n";

        WavInfo info = parseWavInfo(buffer);

        std::cout << "=== Informations sur le fichier WAV ===\n";
        std::cout << "Audio Format : " << readUInt16LE(buffer, 20) << "\n";
        std::cout << "BlockAlign: " << readUInt16LE(buffer, 32) << "\n";
        std::cout << "Canaux : " << info.numChannels << "\n";
        std::cout << "Frequence : " << info.sampleRate << " Hz\n";
        std::cout << "Bits par echantillon : " << info.bitsPerSample << "\n";
        std::cout << "Offset chunk data : " << info.dataChunkOffset << "\n";
        std::cout << "Taille data : " << info.dataSize << " octets\n";
        std::cout << "Debut donnees audio : " << info.audioDataOffset << "\n";

        std::cout << "======Nouveau fichier après division de la fréquence d'echantillonnage par 2 en calculant la moyenne======" << "\n";
        if (info.audioFormat != 1) {
            throw std::runtime_error("Seul le format PCM entier est supporte pour le moment");
        }

        if (info.bitsPerSample != 16) {
            throw std::runtime_error("Seul le PCM 16 bits est supporte pour le moment");
        }

        std::vector<int16_t> samples = extractSamples16Bits(
            buffer,
            info.audioDataOffset,
            info.dataSize
        );

        std::cout << "Nombre total de samples : " << samples.size() << "\n";

        std::vector<int16_t> downsampledSamples = downsampleBy2ByFrames(
            samples,
            info.numChannels
        );

        std::cout << "Nombre de samples apres downsampling : "
                  << downsampledSamples.size()
                  << "\n";

        std::vector<uint8_t> newAudioBytes =
            samples16ToBytes(downsampledSamples);

        std::vector<uint8_t> downsampledBuffer(
            buffer.begin(),
            buffer.begin() + static_cast<long>(info.audioDataOffset)
        );

        downsampledBuffer.insert(
            downsampledBuffer.end(),
            newAudioBytes.begin(),
            newAudioBytes.end()
        );

        updateHeaderAfterDownsamplingBy2(
            downsampledBuffer,
            info,
            static_cast<uint32_t>(newAudioBytes.size())
        );

        writeBinaryFile("output/downsampled.wav", downsampledBuffer);
        std::cout << "Nouveau fichier cree : output/downsampled.wav\n";
        std::cout << "Nouvelle frequence : " << info.sampleRate / 2 << " Hz\n";
        std::cout << "Nouvelle taille data : " << newAudioBytes.size() << " octets\n";

        std::cout << "======Nouveau fichier apres quantification en 8 bits======" << "\n";

        std::vector<uint8_t> quantizedAudio = quantize16To8(samples);

        std::vector<uint8_t> quantizedBuffer(
            buffer.begin(),
            buffer.begin() + static_cast<long>(info.audioDataOffset)
        );

        quantizedBuffer.insert(
            quantizedBuffer.end(),
            quantizedAudio.begin(),
            quantizedAudio.end()
        );

        updateHeaderAfterQuantization8Bits(
            quantizedBuffer,
            info,
            static_cast<uint32_t>(quantizedAudio.size())
        );

        writeBinaryFile("output/quantized_8bit.wav", quantizedBuffer);

        std::cout << "Nouveau fichier cree : output/quantized_8bit.wav\n";
        std::cout << "Nouveau format : PCM 8 bits\n";
        std::cout << "Nouvelle taille data : " << quantizedAudio.size() << " octets\n";

        std::cout << "======Nouveau fichier apres Gestion de la saturation - Soft Limiting ======" << "\n";
        size_t saturatedCount = countSaturatedSamples(samples);

        std::cout << "Samples satures : "
                << saturatedCount
                << "\n";

        std::vector<int16_t> desaturatedSamples =
            softLimit16(samples, 0.95);

        std::vector<uint8_t> desaturatedBytes =
            samples16ToBytes(desaturatedSamples);

        std::vector<uint8_t> desaturatedBuffer = buffer;

        std::copy(
            desaturatedBytes.begin(),
            desaturatedBytes.end(),
            desaturatedBuffer.begin() + static_cast<long>(info.audioDataOffset)
        );

        writeBinaryFile("output/desaturated.wav", desaturatedBuffer);

        std::cout << "Nouveau fichier cree : output/desaturated.wav\n";

        std::cout << "======Normalisation du signal======" << "\n";

        int32_t maxAmplitude = findMaxAmplitude16(samples);

        std::cout << "Amplitude maximale avant normalisation : "
                << maxAmplitude
                << "\n";

        std::vector<int16_t> normalizedSamples =
            normalize16(samples, 0.95);

        std::vector<uint8_t> normalizedBytes =
            samples16ToBytes(normalizedSamples);

        std::vector<uint8_t> normalizedBuffer = buffer;

        std::copy(
            normalizedBytes.begin(),
            normalizedBytes.end(),
            normalizedBuffer.begin() + static_cast<long>(info.audioDataOffset)
        );

        writeBinaryFile("output/normalized.wav", normalizedBuffer);

        std::cout << "Nouveau fichier cree : output/normalized.wav\n";
    }
    catch (const std::exception& error) {
        std::cerr << "Erreur : " << error.what() << "\n";
        return 1;
    }

    return 0;
}