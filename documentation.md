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

