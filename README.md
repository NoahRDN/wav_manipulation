# Manipulation d'un fichier WAV

Ce projet C++ lit un fichier `input.wav`, extrait ses informations techniques, applique plusieurs traitements audio en manipulant directement les octets du format WAV, puis génère de nouveaux fichiers dans `output/`.

## Fonctionnalités

Le programme principal :

- charge `input.wav` en binaire ;
- vérifie la structure `RIFF/WAVE` ;
- lit les métadonnées utiles du header WAV ;
- localise le chunk `data` ;
- extrait les échantillons PCM 16 bits ;
- génère plusieurs fichiers audio transformés.

Les traitements actuellement produits sont :

- `output/downsampled.wav` : sous-échantillonnage par 2 ;
- `output/quantized_8bit.wav` : quantification de 16 bits vers 8 bits ;
- `output/desaturated.wav` : atténuation des saturations par soft limiting ;
- `output/normalized.wav` : normalisation du signal ;
- `output/left_channel.wav` : extraction du canal gauche en mono.

## Structure du projet

- `main.cpp` : point d'entrée du programme
- `src/` : implémentation de la lecture WAV et des traitements audio
- `include/` : fichiers d'en-tête
- `run.sh` : script de compilation et d'exécution du programme principal
- `play.cpp` : lecteur audio simple en C++ avec `SFML Audio`
- `input.wav` : fichier source analysé et transformé
- `output/` : binaires, logs et fichiers WAV générés
- `documentation.md` : notes techniques sur le format WAV et les transformations
- `utilities/` : documents de support

## Prérequis

Pour le programme principal :

- `g++` avec support C++17 ;
- `bash`.

Pour le test auditif en C++ :

- `libsfml-dev`.

Installation sur Ubuntu :

```bash
sudo apt update
sudo apt install libsfml-dev
```

Vérification du compilateur :

```bash
g++ --version
```

## Compilation et exécution

Depuis la racine du projet :

```bash
./run.sh
```

Le script :

1. compile `main.cpp` avec les fichiers de `src/` ;
2. génère le binaire `output/wav_tp` ;
3. exécute le programme ;
4. écrit le journal dans `output/run.log`.

## Résultats générés

Après exécution, vous trouverez notamment :

- `output/run.log` : résumé de l'analyse et des traitements ;
- `output/downsampled.wav`
- `output/quantized_8bit.wav`
- `output/desaturated.wav`
- `output/normalized.wav`
- `output/left_channel.wav`

Pour afficher le journal :

```bash
cat output/run.log
```

## Étape 10 - Test auditif en C++

Pour écouter les fichiers WAV générés en C++, le projet utilise `SFML Audio`, qui fournit un équivalent simple à `pygame` ou `sounddevice` pour charger puis lire un fichier audio.

Le lecteur est déjà fourni dans `play.cpp` et repose sur :

- `sf::SoundBuffer::loadFromFile(...)` pour charger le fichier ;
- `sf::Sound` pour lancer la lecture.

SFML prend notamment en charge les formats :

- `WAV`
- `OGG/Vorbis`
- `FLAC`

### Compiler le lecteur audio

Depuis la racine du projet :

```bash
g++ -std=c++17 play.cpp -o output/play -lsfml-audio -lsfml-system
```

### Tester un fichier généré

```bash
./output/play output/downsampled.wav
```

Autres exemples :

```bash
./output/play output/quantized_8bit.wav
./output/play output/normalized.wav
./output/play output/left_channel.wav
```

## Exemple d'informations affichées

Le programme principal affiche notamment :

```text
Canaux : 2
Frequence : 44100 Hz
Bits par echantillon : 16
Offset chunk data : 244
Taille data : 44347392 octets
Debut donnees audio : 252
```

## Remarque

Le projet est centré sur la manipulation binaire du format WAV. Les nouveaux fichiers sont reconstruits à partir des données transformées et d'un header mis à jour selon le traitement appliqué.


---------------

g++ -std=c++17 -O2 \
    main.cpp \
    src/binary_utils.cpp \
    src/wav_file.cpp \
    src/audio_processing.cpp \
    -Iinclude \
    -o output/wav_tp