#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>



int main() {
    std::ifstream file("input.wav", std::ios::binary);

    if (!file) {
        std::cerr << "Erreur : impossible d'ouvrir input.wav\n";
        return 1;
    }

    std::vector<uint8_t> buffer{
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    };

    std::cout << "Fichier charge : " << buffer.size() << " octets\n";

    return 0;
}
