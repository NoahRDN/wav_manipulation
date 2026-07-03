Oui. Voici le TP **question par question**, avec exactement les trois angles :

1. ce que la question demande réellement ;
2. la logique technique ;
3. le lien avec ton code C++ actuel.

Le PDF impose de lire le fichier en **mode binaire**, de manipuler un tableau d’octets manuellement, et de tenir compte du **Little Endian**. 

---

# Question 1 — Lecture de l’en-tête

## 1. Ce que la question demande

On te demande d’extraire depuis le fichier WAV :

```txt
- fréquence d’échantillonnage
- nombre de canaux
- quantification / bits par échantillon
```

Donc tu dois lire certaines informations du **header WAV**.

## 2. Logique technique

Un WAV contient des informations fixes dans le header :

```txt
offset 22 → NumChannels       sur 2 octets
offset 24 → SampleRate        sur 4 octets
offset 34 → BitsPerSample     sur 2 octets
```

Comme ces nombres sont stockés en **Little Endian**, tu dois utiliser :

```cpp
readUInt16LE(...)
readUInt32LE(...)
```

pour reconstruire les nombres correctement.

## 3. Lien avec ton code

Dans ton code, cette partie est faite dans `parseWavInfo()` :

```cpp
info.audioFormat   = readUInt16LE(buffer, 20);
info.numChannels   = readUInt16LE(buffer, 22);
info.sampleRate    = readUInt32LE(buffer, 24);
info.byteRate      = readUInt32LE(buffer, 28);
info.blockAlign    = readUInt16LE(buffer, 32);
info.bitsPerSample = readUInt16LE(buffer, 34);
```

C’est exactement la réponse à la question 1.

---

# Question 2 — Identification des données audio

## 1. Ce que la question demande

On te demande de trouver où commencent les vraies données audio, c’est-à-dire le chunk :

```txt
data
```

Puis de stocker les échantillons dans un tableau séparé.

## 2. Logique technique

Un chunk WAV a cette structure :

```txt
4 octets → nom du chunk
4 octets → taille du chunk
n octets → données
```

Donc quand tu trouves `"data"` :

```txt
dataChunkOffset       → début du mot "data"
dataChunkOffset + 4   → taille des données audio
dataChunkOffset + 8   → début réel des données audio
```

## 3. Lien avec ton code

Tu as une fonction `findDataChunk()` qui cherche le chunk `"data"` en parcourant les chunks du fichier.

Puis dans `parseWavInfo()` tu fais :

```cpp
info.dataSize = readUInt32LE(buffer, info.dataChunkOffset + 4);
info.audioDataOffset = info.dataChunkOffset + 8;
```

Et ensuite tu extrais les samples avec :

```cpp
extractSamples16Bits(...)
```

qui lit les données audio deux octets par deux octets en PCM 16 bits.

---

# Question 3 — Changement d’échantillonnage

## 1. Ce que la question demande

On te demande de diviser la fréquence d’échantillonnage par 2.

Exemple :

```txt
44100 Hz → 22050 Hz
```

Donc il faut produire deux fois moins de frames audio.

## 2. Logique technique

Pour diviser par 2, on prend deux frames consécutives et on en crée une nouvelle.

En mono :

```txt
S0, S1 → moyenne(S0, S1)
S2, S3 → moyenne(S2, S3)
```

En stéréo :

```txt
L0 R0  L1 R1 → moyenne(L0,L1), moyenne(R0,R1)
```

Il faut faire la moyenne **canal par canal**, sinon on mélange gauche et droite.

Ensuite, il faut mettre à jour le header :

```txt
SampleRate = ancien SampleRate / 2
ByteRate   = SampleRate × BlockAlign
DataSize   = nouvelle taille audio
ChunkSize  = nouvelle taille fichier - 8
```

## 3. Lien avec ton code

Ton code utilise déjà la bonne méthode avec :

```cpp
downsampleBy2ByFrames(samples, info.numChannels)
```

Cette fonction prend deux frames consécutives et calcule la moyenne pour chaque canal.

Puis tu mets à jour le header avec :

```cpp
updateHeaderAfterDownsamplingBy2(...)
```

qui modifie `SampleRate`, `ByteRate`, `DataSize` et `ChunkSize`.

---

# Question 4 — Changement de quantification

## 1. Ce que la question demande

On te demande de réduire la résolution du son, par exemple :

```txt
16 bits → 8 bits
```

Donc chaque sample ne fait plus 2 octets, mais 1 octet.

## 2. Logique technique

En PCM 16 bits signé :

```txt
-32768 à 32767
```

En PCM 8 bits WAV classique :

```txt
0 à 255
```

Donc on transforme :

```txt
sample8 = (sample16 + 32768) / 256
```

ou équivalent :

```txt
sample8 = (sample16 + 32768) >> 8
```

Cela conserve la forme générale de l’onde, mais avec moins de précision.

Il faut mettre à jour :

```txt
BitsPerSample = 8
BlockAlign    = NumChannels × BitsPerSample/8
ByteRate      = SampleRate × BlockAlign
DataSize      = nouvelle taille audio
ChunkSize     = nouvelle taille fichier - 8
```

## 3. Lien avec ton code

Tu as déjà la fonction :

```cpp
quantize16To8(samples)
```

Elle applique la formule :

```cpp
(sample16 + 32768) / 256
```

Puis tu mets à jour le header avec :

```cpp
updateHeaderAfterQuantization8Bits(...)
```

Cette fonction met `BitsPerSample` à `8`, recalcule `BlockAlign`, `ByteRate`, `DataSize` et `ChunkSize`.

---

# Question 5 — Gestion de la saturation

## 1. Ce que la question demande

On te demande de repérer les samples qui atteignent les limites du format.

En PCM 16 bits :

```txt
32767  → limite positive
-32768 → limite négative
```

Puis on doit corriger sans couper brutalement l’onde.

## 2. Logique technique

Un écrêtage brutal ferait :

```txt
si sample > 32767 → sample = 32767
si sample < -32768 → sample = -32768
```

Mais le TP demande d’éviter cette coupure dure.

Donc on applique un **soft limiter** :

```txt
petites amplitudes → presque inchangées
grosses amplitudes → compressées progressivement
```

Ici, le header ne change pas, car on ne modifie ni la fréquence, ni le nombre de canaux, ni la quantification, ni la taille des données.

## 3. Lien avec ton code

Tu comptes d’abord les samples saturés avec :

```cpp
countSaturatedSamples(samples)
```

Puis tu appliques :

```cpp
softLimit16(samples, 0.95)
```

Cette fonction compresse progressivement les valeurs proches des limites avec `std::tanh`.

---

# Question 6 — Normalisation

## 1. Ce que la question demande

On te demande de trouver l’amplitude maximale du signal, puis d’appliquer un gain global pour rendre le son plus fort sans dépasser la saturation.

## 2. Logique technique

On cherche :

```txt
maxAmplitude = max(abs(sample))
```

Puis on calcule :

```txt
gain = amplitudeCible / maxAmplitude
```

Exemple :

```txt
amplitudeCible = 32767 × 0.95
```

Puis :

```txt
nouveauSample = sample × gain
```

Le header ne change pas, car on modifie seulement les valeurs audio.

## 3. Lien avec ton code

Tu as :

```cpp
findMaxAmplitude16(samples)
```

qui cherche la plus grande amplitude en valeur absolue.

Puis :

```cpp
normalize16(samples, 0.95)
```

qui applique un gain global pour atteindre 95 % de l’amplitude maximale autorisée.

---

# Question 7 — Extraction de canaux

## 1. Ce que la question demande

Si le fichier est stéréo, on te demande d’isoler le canal gauche.

Un fichier stéréo est entrelacé :

```txt
L0 R0 L1 R1 L2 R2 ...
```

## 2. Logique technique

Pour extraire le canal gauche, on garde :

```txt
L0, L1, L2, ...
```

Donc on prend le canal d’index `0` dans chaque frame.

Le nouveau fichier devient mono :

```txt
NumChannels = 1
```

Il faut mettre à jour :

```txt
NumChannels
BlockAlign
ByteRate
DataSize
ChunkSize
```

## 3. Lien avec ton code

Tu as :

```cpp
extractChannel16(samples, info.numChannels, 0)
```

Le `0` correspond au canal gauche.

Puis tu mets à jour le header avec :

```cpp
updateHeaderAfterMonoExtraction(...)
```

qui passe `NumChannels` à `1` et recalcule les champs nécessaires.

---

# Question 9 — Sauvegarde du fichier

## 1. Ce que la question demande

On te demande de reconstruire un fichier `.wav` valide après traitement.

Donc il faut écrire :

```txt
header valide + données audio modifiées
```

## 2. Logique technique

Selon le traitement, il faut recalculer :

```txt
ChunkSize
DataSize
ByteRate
BlockAlign
SampleRate
BitsPerSample
NumChannels
```

Mais pas tous à chaque fois. Ça dépend du traitement.

Ensuite, tu écris le buffer final en mode binaire.

## 3. Lien avec ton code

Tu as :

```cpp
writeBinaryFile(...)
```

qui écrit un `std::vector<uint8_t>` dans un fichier `.wav`.

Tu reconstruis aussi les buffers de sortie dans `main.cpp`, par exemple pour `downsampled.wav`, `quantized_8bit.wav`, `left_channel.wav`, etc.

---

# Question 10 — Test auditif

## 1. Ce que la question demande

On te demande d’écouter le fichier généré pour vérifier que le résultat est bien lisible.

Le PDF propose des bibliothèques simples comme `sounddevice` ou `pygame`. 

## 2. Logique technique

Le test auditif sert à vérifier :

```txt
- le fichier s’ouvre correctement
- le header est valide
- le son correspond au traitement attendu
```

## 3. Lien avec ton code

Tu as utilisé un petit programme de lecture avec SFML, ou directement VLC.

C’est acceptable : VLC confirme que tes fichiers `.wav` générés sont lisibles. Pour `stereo_to_21.wav`, tu as aussi constaté que VLC le lit même si certains lecteurs supportent mal le WAV 3 canaux.

---

# Question 11 — Passage stéréo 2.0 vers 2.1

## 1. Ce que la question demande

On te demande de partir d’un fichier stéréo :

```txt
L R
```

et d’ajouter un troisième canal :

```txt
Sub / LFE
```

Donc la nouvelle structure devient :

```txt
[L, R, Sub]
```

## 2. Logique technique

Pour chaque frame :

```txt
Sub = (L + R) / 2
```

Puis tu réentrelaces :

```txt
L0 R0 Sub0  L1 R1 Sub1 ...
```

Pour un vrai canal LFE, on peut appliquer un filtre passe-bas au canal Sub pour garder surtout les graves.

Il faut mettre à jour :

```txt
NumChannels = 3
BlockAlign
ByteRate
DataSize
ChunkSize
```

## 3. Lien avec ton code

Tu as :

```cpp
stereoTo21(samples, info.numChannels, true, 0.08)
```

Cette fonction ajoute `left`, `right`, puis `subValue`, donc l’ordre `[L, R, Sub]` est bien respecté.

Le filtre passe-bas simple est appliqué avec :

```cpp
previousSub = alpha * subValue + (1.0 - alpha) * previousSub;
```

Puis ton header est mis à jour avec :

```cpp
updateHeaderAfterStereoTo21(...)
```

qui passe `NumChannels` à `3`.

---

# Question 12 — Simulation surround 5.1

## 1. Ce que la question demande

On te demande de créer un fichier 6 canaux à partir d’une source stéréo.

Structure demandée :

```txt
[L, R, C, LFE, Ls, Rs]
```

## 2. Logique technique

À partir de :

```txt
L, R
```

on crée :

```txt
L   = L original
R   = R original
C   = L + R
LFE = (L + R) / 2
Ls  = L / 2
Rs  = R / 2
```

Comme `L + R` peut dépasser les limites du `int16_t`, il faut limiter avec un clamp.

Le nouveau header doit contenir :

```txt
NumChannels = 6
BlockAlign = 6 × BitsPerSample/8
ByteRate = SampleRate × BlockAlign
```

## 3. Lien avec ton code

Tu as :

```cpp
stereoTo51(samples, info.numChannels)
```

Cette fonction crée bien les 6 canaux dans l’ordre :

```txt
L, R, C, LFE, Ls, Rs
```

avec `C = L + R`, `LFE = (L + R)/2`, `Ls = L/2`, `Rs = R/2`.

Et tu mets à jour le header avec :

```cpp
updateHeaderAfterStereoTo51(...)
```

qui passe `NumChannels` à `6` et recalcule `BlockAlign`, `ByteRate`, `DataSize` et `ChunkSize`.

---

# Question 13 — Génération d’onde sinusoïdale 440 Hz

## 1. Ce que la question demande

On te demande de générer toi-même un tableau d’octets représentant une onde sinusoïdale pure de fréquence :

```txt
440 Hz
```

pendant :

```txt
1 seconde
```

Puis, en bonus, de faire voyager ce son dans un fichier 5.1 en variant l’amplitude dans les différents canaux.

## 2. Logique technique

Pour générer une onde, on utilise :

```txt
sample[n] = amplitude × sin(2π × fréquence × n / SampleRate)
```

Pour 1 seconde à `44100 Hz`, tu génères :

```txt
44100 frames
```

Pour le voyage en 5.1, tu crées 6 canaux et tu fais varier les gains de canal au fil du temps.

## 3. Lien avec ton code

Tu as :

```cpp
generateSineWave16(...)
```

qui génère une onde sinusoïdale mono 16 bits.

Tu as aussi :

```cpp
generateTravelingSine51(...)
```

qui génère une onde 440 Hz sur 6 canaux avec déplacement progressif d’un canal à l’autre.

Comme ici tu ne pars pas de `input.wav`, tu crées un WAV depuis zéro avec :

```cpp
buildWavPcmBuffer(...)
```

Cette fonction construit un header RIFF/WAVE/fmt/data complet, puis ajoute les bytes audio.

---

# Résumé final

Ton projet couvre déjà toutes les questions principales du TP :

```txt
1  Lecture header                     → parseWavInfo()
2  Data chunk                         → findDataChunk()
3  Sous-échantillonnage               → downsampleBy2ByFrames()
4  Quantification 16 → 8 bits          → quantize16To8()
5  Saturation                         → countSaturatedSamples(), softLimit16()
6  Normalisation                      → findMaxAmplitude16(), normalize16()
7  Extraction canal gauche            → extractChannel16()
9  Export WAV                         → writeBinaryFile(), updateHeader...
10 Test auditif                       → VLC / SFML
11 Stéréo vers 2.1                    → stereoTo21()
12 Stéréo vers 5.1                    → stereoTo51()
13 Synthèse 440 Hz                    → generateSineWave16(), generateTravelingSine51()
```

La seule anomalie du PDF est l’absence de la question 8.
