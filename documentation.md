Dans un fichier .wav, les données sont stockées en binaire. Pour les lire, on copie les octets bruts du fichier dans un tableau de uint8_t. Donc chaque offset du tableau correspond à 1 octet, soit 8 bits.

Dans ce tableau, il y a plusieurs zones : le header, les chunks, puis les données audio.

Pour lire les informations du header, on utilise les offsets définis par le format WAV. Si l’information est un nombre stocké sur plusieurs octets, on utilise le Little Endian pour reconstruire la valeur.

offset 0 à 3   : "RIFF"
offset 8 à 11  : "WAVE"

offset 22, taille 2 bytes : nombre de canaux
offset 24, taille 4 bytes : fréquence d’échantillonnage
offset 34, taille 2 bytes : bits par échantillon

Pour trouver les données audio, on cherche le chunk "data".
Quand on trouve l’offset de "data" :
- offset + 4 contient la taille des données audio sur 4 octets ;
- offset + 8 correspond au début réel des données audio.

dans le cas où on veux diviser la fréquence
d'échantillonnage par 2 en calculant la moyenne (ou le max) de deux échantillons
consécutifs, pour que le fichier reste visible, une mise a jour de l'en tête est obligatoire dont le procéder est comme suit: 
- il y a ces 4 éléments qui sont les strict minimum a reajuster par consequent: 
    -SampleRate: 
        - nombre de sample par seconde
        - permet au lecteur de savoir la vitesse temporelle du son 
        - dans notre cas newSampleRate = SampleRate / 2
    - ByteRate: 
        - nombre d'octet lu par seconde
        - formule: SampleRate × NumChannels × BitsPerSample/8

SampleRate = vitesse temporelle
BitsPerSample = taille/précision d’un sample
NumChannels = nombre de pistes audio
BlockAlign = taille d’un instant audio complet
ByteRate = nombre d’octets lus par seconde
DataSize = taille du bloc audio brut

Si on divise la fréquence d’échantillonnage par 2 :

SampleRate change
ByteRate change
DataSize change
ChunkSize change
BitsPerSample ne change pas
NumChannels ne change pas
BlockAlign ne change pas

Si on change la quantification de 16 bits à 8 bits :

BitsPerSample change
BlockAlign change
ByteRate change
DataSize change
ChunkSize change
SampleRate ne change pas
NumChannels ne change pas

Un sample = valeur d’un canal.
Une frame audio = ensemble des samples de tous les canaux à un instant donné.

Après le sous-échantillonnage, le nombre de frames audio est divisé par 2. Il faut donc diviser le champ SampleRate par 2. Le champ ByteRate doit être recalculé avec la formule SampleRate × NumChannels × BitsPerSample/8. Comme le tableau de données audio devient plus petit, il faut aussi mettre à jour la taille du chunk data, c’est-à-dire Subchunk2Size/DataSize. Enfin, la taille globale RIFF ChunkSize doit être recalculée avec la nouvelle taille du fichier moins 8 octets. Les champs NumChannels, BitsPerSample, BlockAlign et AudioFormat ne changent pas si on ne modifie ni le nombre de canaux ni la quantification.

Tu peux répondre comme ça :

Pour réduire la quantification de **16 bits à 8 bits**, il faut transformer chaque amplitude 16 bits en une amplitude 8 bits en gardant une correspondance proportionnelle entre les valeurs.

Comme un sample PCM 16 bits signé varie généralement de :

```txt
-32768 à 32767
```

et qu’un sample PCM 8 bits WAV est généralement non signé, donc :

```txt
0 à 255
```

on peut utiliser l’opération arithmétique suivante :

```txt
sample8 = (sample16 + 32768) / 256
```

ou, de manière équivalente avec un décalage binaire :

```txt
sample8 = (sample16 + 32768) >> 8
```

Cette opération décale d’abord le signal pour passer de l’intervalle signé `[-32768 ; 32767]` vers `[0 ; 65535]`, puis réduit la résolution de 16 bits à 8 bits.

Exemple :

```txt
sample16 = -32768  → sample8 = 0
sample16 = 0       → sample8 = 128
sample16 = 32767   → sample8 ≈ 255
```

Donc la forme générale de l’onde est conservée, car toutes les amplitudes sont transformées de manière proportionnelle. On perd seulement de la précision, pas la structure globale du signal.


Dans un fichier WAV stéréo, les samples sont entrelacés sous la forme L0 R0 L1 R1 L2 R2. Pour isoler le canal gauche, on parcourt les frames audio et on conserve uniquement le premier sample de chaque frame, c’est-à-dire L0, L1, L2, etc. Le nouveau fichier devient mono, donc il faut mettre à jour le header : NumChannels passe à 1, BlockAlign devient NumChannels × BitsPerSample/8, ByteRate devient SampleRate × BlockAlign, DataSize devient la taille du nouveau tableau audio, et ChunkSize est recalculé avec la nouvelle taille du fichier moins 8.

Après chaque traitement, je reconstruis un fichier WAV en conservant l’en-tête original jusqu’au début des données audio, puis j’insère le nouveau tableau d’octets audio. Les champs du header dépendant de la transformation sont recalculés, notamment ChunkSize, DataSize, ByteRate, SampleRate, BlockAlign, BitsPerSample ou NumChannels selon le cas. Le fichier final est ensuite écrit en mode binaire dans un nouveau fichier .wav.

Pour valider le résultat, j’utilise une bibliothèque de lecture audio simple, comme SFML Audio en C++, afin de charger le fichier WAV généré et de l’écouter.

Sans filtre :

Sub = (L + R) / 2

Mais ce canal contient encore toutes les fréquences : graves, médiums, aigus.

Pour créer un vrai canal LFE, il faudrait garder surtout les basses fréquences. Ici, on utilise un filtre simple :

previousSub = alpha * subValue + (1.0 - alpha) * previousSub;

C’est un filtre passe-bas très simple. Il lisse le signal, donc il atténue une partie des variations rapides, qui correspondent souvent aux fréquences plus aiguës.

Plus alpha est petit, plus le signal est lissé.

Exemple :

alpha = 0.08

donne un canal Sub plus doux.

Pour passer d’un fichier stéréo 2.0 à un fichier 2.1, je parcours les frames audio du fichier original. Chaque frame contient un sample gauche L et un sample droit R. Je conserve ces deux samples, puis je crée un troisième sample Sub en calculant la moyenne : Sub = (L + R) / 2. Le nouveau tableau audio est ensuite ré-entrelacé selon le schéma [L, R, Sub].

Pour obtenir un canal LFE plus réaliste, on peut appliquer un filtre passe-bas sur le canal Sub afin d’atténuer les fréquences aiguës et de conserver principalement les graves. Après la transformation, le header WAV doit être mis à jour : NumChannels passe à 3, BlockAlign devient 3 × BitsPerSample/8, ByteRate devient SampleRate × BlockAlign, DataSize devient la taille du nouveau tableau audio, et ChunkSize est recalculé avec la taille totale du fichier moins 8.

Pour simuler un fichier surround 5.1 à partir d’une source stéréo, je parcours les frames du fichier original. Chaque frame contient deux samples : L et R. Je crée ensuite une nouvelle frame de 6 canaux selon l’ordre [L, R, C, LFE, Ls, Rs]. Les canaux L et R conservent les valeurs originales. Le canal central C reçoit la somme de L et R, avec une limitation pour éviter le dépassement des bornes du format 16 bits. Le canal LFE reçoit la moyenne de L et R. Les canaux arrière Ls et Rs reçoivent respectivement L/2 et R/2 pour obtenir une version atténuée du signal.

Après cette transformation, le header WAV doit être mis à jour : NumChannels passe à 6, BlockAlign devient 6 × BitsPerSample/8, ByteRate devient SampleRate × BlockAlign, DataSize devient la taille du nouveau tableau audio, et ChunkSize est recalculé avec la taille totale du fichier moins 8.


# les informations entre l’offset 4 à 7

Les octets :

offset 4, 5, 6, 7

contiennent le champ :



C’est une valeur sur 4 octets, donc un uint32_t.

Son rôle : indiquer la taille du fichier WAV à partir de l’offset 8.

Autrement dit :

ChunkSize = taille totale du fichier - 8

Donc si ton fichier fait par exemple :

100 000 octets

alors normalement :

ChunkSize = 99 992

Pourquoi -8 ? Parce qu’on ne compte pas :

offset 0-3 : "RIFF"
offset 4-7 : ChunkSize lui-même

Donc ChunkSize décrit ce qui vient après ces 8 premiers octets.

Dans ton code, tu lis cette valeur avec :

readUInt32LE(buffer, 4)

# Le rôle de cette partie dans findDataChunk

```
offset += 8 + chunkSize;

if (chunkSize % 2 != 0) {
    offset += 1;
}
```
# Vue d’ensemble du header WAV classique

|    Offset | Nombre d’octets | Nom du champ    | Rôle                                                |
| --------: | --------------: | --------------- | --------------------------------------------------- |
|   `0 - 3` |               4 | `ChunkID`       | Doit contenir `"RIFF"`                              |
|   `4 - 7` |               4 | `ChunkSize`     | Taille du fichier moins 8                           |
|  `8 - 11` |               4 | `Format`        | Doit contenir `"WAVE"`                              |
| `12 - 15` |               4 | `Subchunk1ID`   | Doit contenir `"fmt "`                              |
| `16 - 19` |               4 | `Subchunk1Size` | Taille du chunk `fmt`, souvent `16` pour PCM simple |
| `20 - 21` |               2 | `AudioFormat`   | Type d’encodage audio, `1` = PCM                    |
| `22 - 23` |               2 | `NumChannels`   | Nombre de canaux : `1` mono, `2` stéréo, `6` 5.1    |
| `24 - 27` |               4 | `SampleRate`    | Fréquence d’échantillonnage, ex. `44100` Hz         |
| `28 - 31` |               4 | `ByteRate`      | Nombre d’octets lus par seconde                     |
| `32 - 33` |               2 | `BlockAlign`    | Nombre d’octets par frame audio                     |
| `34 - 35` |               2 | `BitsPerSample` | Taille d’un sample : ex. `16 bits`                  |
| `36 - 39` |               4 | `Subchunk2ID`   | Souvent `"data"` dans un WAV simple                 |
| `40 - 43` |               4 | `Subchunk2Size` | Taille des données audio                            |
|  `44 ...` |        variable | Audio data      | Les samples audio réels                             |


# C’est quoi un chunk réellement ?
Un chunk, c’est un bloc structuré dans un fichier WAV.

Tu peux imaginer un fichier WAV comme une succession de boîtes :

[RIFF]
    [fmt ]
    [data]
    [autres chunks possibles]

Chaque boîte a :

1. un nom
2. une taille
3. un contenu

En binaire, un chunk WAV ressemble à ça :

4 octets → nom du chunk
4 octets → taille du contenu
n octets → contenu réel du chunk

Exemple avec le chunk fmt :

offset 12 - 15 : "fmt "
offset 16 - 19 : taille du chunk fmt
offset 20 - 35 : contenu du chunk fmt

Donc le chunk fmt n’est pas seulement les lettres "fmt ". Le chunk complet contient aussi sa taille et son contenu.

## Exemple concret avec un WAV simple

Dans un WAV classique, tu peux avoir :

offset 0  - 3  : "RIFF"
offset 4  - 7  : taille du fichier - 8
offset 8  - 11 : "WAVE"

offset 12 - 15 : "fmt "
offset 16 - 19 : taille du chunk fmt = 16
offset 20 - 35 : contenu du chunk fmt

offset 36 - 39 : "data"
offset 40 - 43 : taille des données audio
offset 44 ...  : samples audio

Donc ici :

le chunk fmt commence à offset 12
sa taille de contenu est 16
le prochain chunk commence à offset 12 + 8 + 16 = 36

Voilà pourquoi on fait :

offset += 8 + chunkSize;
## Pourquoi on saute 8 + chunkSize ?

Parce que le chunk est composé de :

4 octets : nom du chunk
4 octets : taille du chunk
chunkSize octets : contenu du chunk

Donc pour passer au prochain chunk, il faut sauter tout le chunk actuel.

taille totale du chunk = 4 + 4 + chunkSize
                      = 8 + chunkSize

Exemple :

offset actuel = 12
chunk actuel = "fmt "
chunkSize = 16

Donc :

prochain chunk = 12 + 8 + 16
               = 36

À l’offset 36, on trouve souvent :

"data"
## Pourquoi data ne peut pas simplement être à offset + 1 ?

Techniquement, les lettres "data" peuvent apparaître n’importe où dans les octets du fichier, même à l’intérieur d’un contenu audio.

Mais ça ne veut pas dire que c’est un vrai chunk data.

Un vrai chunk doit commencer exactement à une position structurée comme ceci :

4 octets : nom du chunk
4 octets : taille
n octets : contenu

Si tu fais :

offset + 1

tu arrives au milieu du chunk actuel.

Exemple :

offset 12 : 'f'
offset 13 : 'm'
offset 14 : 't'
offset 15 : ' '

Si tu fais offset + 1, tu arrives à :

offset 13 : 'm'

Tu es au milieu du nom "fmt ". Ce n’est pas un début de chunk.

Donc dans un fichier WAV, on ne cherche pas le chunk suivant octet par octet. On suit la structure officielle :

début du chunk actuel
+ 8 octets de header du chunk
+ taille du contenu du chunk
= début du chunk suivant

C’est exactement ce que fait ta fonction findDataChunk(). Elle commence à l’offset 12, lit le nom du chunk, lit sa taille, puis saute au chunk suivant jusqu’à trouver "data".

## remarque: 
Donc dans un fichier WAV, on ne cherche pas le chunk suivant octet par octet. On suit la structure officielle :

début du chunk actuel
+ 8 octets de header du chunk
+ taille du contenu du chunk
= début du chunk suivant

# C’est quoi un sample ?

Un sample, c’est une valeur numérique qui représente l’amplitude du son à un instant précis.
Le son réel est une onde continue. L’ordinateur ne peut pas stocker une onde continue directement, donc il prend plein de mesures très rapidement.

Exemple avec une fréquence d’échantillonnage de 44100 Hz :

44100 samples par seconde en mono

Chaque sample est une mesure du signal audio.

En PCM 16 bits, un sample peut avoir une valeur entre :

-32768 et 32767

Exemple :

sample 0 = 0
sample 1 = 1200
sample 2 = 2500
sample 3 = -1000
sample 4 = -3000

# Sample en mono vs stéréo

En mono, chaque instant audio contient un seul sample :

S0 S1 S2 S3 ...

En stéréo, chaque instant audio contient deux samples :

L0 R0  L1 R1  L2 R2 ...

Donc :

L0 = sample gauche au temps 0
R0 = sample droite au temps 0
L1 = sample gauche au temps 1
R1 = sample droite au temps 1

Un groupe complet L0 R0 s’appelle une frame audio.

Une frame audio, c’est l’ensemble des samples de tous les canaux à un instant précis.