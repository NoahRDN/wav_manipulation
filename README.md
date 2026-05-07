# Manipulation d'un fichier WAV

Ce projet est un petit programme C++ qui lit un fichier `input.wav` en binaire et affiche quelques informations extraites de son en-tête WAV.

## Ce que fait le projet

Le programme :

- ouvre `input.wav` en mode binaire ;
- vérifie que le fichier est bien au format `RIFF/WAVE` ;
- lit certaines métadonnées du fichier WAV ;
- recherche la section `data` ;
- affiche les informations trouvées dans la sortie standard.

Les informations extraites sont :

- le nombre de canaux ;
- la fréquence d'échantillonnage ;
- le nombre de bits par échantillon ;
- l'offset du chunk `data` ;
- la taille des données audio ;
- la position de début des données audio.

## Structure du projet

- `main.cpp` : code source principal
- `run.sh` : script de compilation et d'exécution
- `input.wav` : fichier audio analysé
- `output/` : dossier contenant le binaire compilé et le journal d'exécution
- `utilities/` : documents de support

## Prérequis

Il faut disposer de :

- `g++` avec support C++17 ;
- `bash`.

Sous Linux, vous pouvez vérifier la présence du compilateur avec :

```bash
g++ --version
```

## Lancer le projet

Depuis la racine du projet :

```bash
./run.sh
```

Le script :

1. compile `main.cpp` ;
2. génère le binaire `output/wav_tp` ;
3. exécute le programme ;
4. enregistre la sortie dans `output/run.log`.

## Consulter le résultat

Après l'exécution, les fichiers utiles sont :

- `output/wav_tp` : le programme compilé
- `output/run.log` : le résultat de l'analyse

Pour afficher le journal :

```bash
cat output/run.log
```

## Exemple d'informations affichées

Le programme affiche notamment :

```text
Canaux : 2
Frequence : 44100 Hz
Bits par echantillon : 16
Offset chunk data : 244
Taille data : 44347392 octets
Debut donnees audio : 252
```

## Remarque

Le projet est orienté lecture et analyse du format WAV. Il ne modifie pas le fichier audio ; il inspecte uniquement sa structure binaire pour en extraire des informations techniques.
