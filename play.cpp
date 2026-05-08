#include <SFML/Audio.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage : ./play output/fichier.wav\n";
        return 1;
    }

    sf::SoundBuffer buffer;

    if (!buffer.loadFromFile(argv[1])) {
        std::cerr << "Erreur : impossible de charger le fichier audio\n";
        return 1;
    }

    sf::Sound sound(buffer);
    sound.play();

    while (sound.getStatus() == sf::Sound::Status::Playing) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}