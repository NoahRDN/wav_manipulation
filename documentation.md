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