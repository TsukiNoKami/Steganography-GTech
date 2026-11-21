[README_Steganography_GTech_final.md](https://github.com/user-attachments/files/35193537/README_Steganography_GTech_final.md)

**Andy Donero & Benjamin Bodin**  
**Gaming Campus — Novembre 2026 — C++20**

Dans ce projet, notre objectif était de cacher un message dans une image BMP, puis de pouvoir le récupérer sans rendre l'image illisible. Nous avons utilisé la méthode LSB (*Least Significant Bit*) : elle consiste à modifier uniquement le dernier bit des composantes couleur. Nous avons également ajouté une option de chiffrement XOR avec une clé.

Le programme fonctionne en ligne de commande. Il accepte des images BMP 24 bits non compressées et n'utilise aucune bibliothèque externe pour les lire ou les modifier.

## 1. Répartition et organisation du projet

Notre projet est divisé en plusieurs fichiers pour séparer la lecture de l'image, la stéganographie, le XOR et les arguments de la console.

| Fichier | Rôle |
| --- | --- |
| `Bmp.h` / `Bmp.cpp` | Ouvrir, vérifier et enregistrer les images BMP |
| `Steganography.h` / `Steganography.cpp` | Insérer et extraire les messages avec les bits LSB |
| `XOR.h` / `XOR.cpp` | Chiffrer et déchiffrer le message avec une clé répétée |
| `CommandLine.h` / `CommandLine.cpp` | Vérifier les arguments et lancer les commandes |
| `ProjetStéganography.cpp` | Point d'entrée du programme (`main`) |
| `ProjetStéganography.slnx` et `.vcxproj` | Solution et configuration Visual Studio |

**Travail réalisé pendant le développement :** Andy a travaillé sur les modules BMP, stéganographie et ligne de commande, leur intégration et les vérifications ; Benjamin a contribué au module XOR. La répartition détaillée et la vérification croisée restent à confirmer ensemble avant le rendu.

---

## 2. Lecture et sauvegarde des images BMP

### Formats acceptés

Notre classe `Bmp` accepte uniquement des images possédant :

- la signature `BM` ;
- un en-tête `BITMAPINFOHEADER` de **40 octets** ;
- **24 bits par pixel**, sans compression ;
- une largeur et une hauteur strictement positives ;
- un décalage vers les pixels valide et suffisamment de données dans le fichier.

Les images dans d'autres formats sont refusées. Nous vérifions aussi la taille déclarée dans l'en-tête et les dimensions pour éviter de lire en dehors du fichier.

### Chargement

La fonction `readFile()` ouvre le fichier avec `fopen()` en mode `rb`, puis lit les données par blocs de **500 octets** grâce à `fread()`. Les octets sont ajoutés dans un `std::vector<std::uint8_t>`.

Contrairement à un découpage qui stockerait les en-têtes et les pixels dans plusieurs tableaux, notre classe `Bmp` conserve **le contenu complet du fichier** dans `bytes`. Elle garde également ces informations :

| Attribut | Utilité |
| --- | --- |
| `width` | Largeur de l'image |
| `height` | Hauteur de l'image |
| `pixelOffset` | Adresse du début des données des pixels |
| `rowStride` | Taille totale d'une ligne, padding compris |
| `bytes` | Octets originaux du fichier BMP |

La fonction `readLittleEndian()` sert à reconstruire les valeurs numériques des en-têtes. Le BMP stocke les entiers multioctets avec l'octet de poids faible en premier.

### Calcul du padding

Dans un BMP 24 bits, chaque pixel contient trois octets dans l'ordre **BGR** : bleu, vert, rouge. Chaque ligne doit occuper un nombre d'octets multiple de quatre.

Par exemple, pour une largeur de cinq pixels :

```text
5 × 3 = 15 octets de couleur
Stride = 16 octets
Padding = 1 octet par ligne
```

Notre calcul dans `calculateStride()` est :

```cpp
const std::uint64_t rowBytes = static_cast<std::uint64_t>(imageWidth) * 3;
return ((rowBytes + 3) / 4) * 4;
```

La méthode `componentOffset()` utilise ensuite `rowStride` pour passer à la ligne suivante sans traiter les octets de padding comme des couleurs. Cela permet de laisser ces octets inchangés.

### Sauvegarde

`Bmp::save()` écrit le tableau complet d'octets avec `fwrite()`. La sauvegarde est ouverte avec le mode `wbx` : **un fichier existant n'est pas écrasé**, ce qui protège aussi l'image source. Si l'écriture ou la fermeture échoue, le programme tente de supprimer le fichier de sortie incomplet.

Sans message à insérer, une lecture suivie d'une sauvegarde doit conserver les octets du BMP.

---

## 3. Insertion d'un message avec le LSB

La fonction `embedMessage()` cache les octets du texte dans les composantes couleur de l'image. Pour chaque composante, on remplace uniquement son bit de poids faible :

```cpp
bytes[offset] = static_cast<std::uint8_t>(
    (bytes[offset] & 0xFEu) | bit);
```

- `& 0xFEu` met le dernier bit à zéro, sans modifier les sept autres ;
- `| bit` insère le bit du message, qui vaut `0` ou `1`.

Un octet de texte possède huit bits. Nous utilisons donc **huit composantes couleur pour cacher un octet**. La fonction `writeByte()` traite les bits du numéro **0 au numéro 7** (du poids faible au poids fort), et avance la position après chaque bit.

### Les métadonnées

Avant le message, on cache neuf octets qui permettent de le retrouver :

| Ordre | Information | Taille |
| --- | --- | ---: |
| 1 | Signature `STEG` | 4 octets |
| 2 | Longueur du message en little-endian | 4 octets |
| 3 | Indicateur XOR (`0` ou `1`) | 1 octet |
| 4 | Message | Taille variable |

Le drapeau vaut `0` si le message est en clair et `1` s'il a été transformé par XOR. Il est important de garder **le même ordre** à l'insertion et à l'extraction. Notre ordre est différent de celui utilisé dans certains autres projets.

### Vérification de la capacité

Nous cachons un bit par composante, soit trois bits par pixel. La capacité en octets se calcule ainsi :

```text
Capacité brute = (largeur × hauteur × 3) / 8
Capacité utile = capacité brute - 9 octets de métadonnées
```

La division est entière. Le programme vérifie d'abord qu'il y a de la place pour les neuf octets de métadonnées, puis que le message ne dépasse pas la place restante. La longueur du message doit aussi tenir sur 32 bits.

Exemple : pour une image de **8 × 8 pixels**, on obtient 24 octets bruts, dont **15 octets pour le texte**.

---

## 4. Extraction du message

La fonction `extractMessage()` réalise le parcours inverse :

1. Elle vérifie que l'image a assez de place pour contenir les métadonnées.
2. Elle relit la signature et refuse l'image si ce n'est pas `STEG`.
3. Elle récupère la longueur enregistrée sur quatre octets.
4. Elle lit le drapeau XOR et vérifie qu'il vaut `0` ou `1`.
5. Elle vérifie que la longueur annoncée ne dépasse pas la capacité disponible.
6. Elle récupère exactement le nombre d'octets prévu avec `readPayload()`.
7. Si le message est chiffré, elle utilise la clé pour retrouver le texte original.

La lecture de chaque bit utilise `& 1u`. La fonction `readByte()` reconstruit ensuite les octets avec des décalages vers la gauche. Le message est conservé sous forme d'octets : il n'y a pas de conversion d'encodage pendant l'insertion ou l'extraction.

---

## 5. Chiffrement XOR — partie de Benjamin

Le module XOR contient `xorCrypt()` et `xorDecrypt()`. Le déchiffrement réutilise le même calcul que le chiffrement, car appliquer deux fois le XOR avec la même clé restitue les données d'origine.

Le traitement parcourt chaque octet du texte et le combine avec un octet de la clé. Le modulo permet de répéter la clé jusqu'à la fin du message :

```cpp
result[i] = message[i] ^ key[i % key.size()];
```

Par exemple, avec la clé `ABC`, on utilise successivement `A`, `B`, `C`, puis de nouveau `A`, `B`, `C`, etc.

La ligne de commande refuse une clé explicitement vide. Le programme fonctionne aussi sans clé : le message est alors inséré sans XOR.

**Limite importante :** ce XOR à clé répétée n'est pas un chiffrement sécurisé. Une mauvaise clé peut produire un texte incorrect sans erreur détectable, ce qui est prévu par le cahier des charges.

---

## 6. Ligne de commande

La fonction `main()` appelle `runApplication()`. Le module `CommandLine.cpp` vérifie la commande et le nombre d'arguments, lit éventuellement la clé, puis lance `executeEmbed()` ou `executeExtract()`.

### Compiler et lancer

Le projet utilise **C++20** et Visual Studio. Pour le compiler :

1. Ouvrir `ProjetStéganography.slnx`.
2. Sélectionner **Debug** et **x64**.
3. Lancer **Générer la solution** (`Ctrl + Maj + B`).
4. Depuis un CMD ouvert dans le dossier du projet, utiliser l'exécutable `x64\Debug\ProjetStéganography.exe`.

Le projet versionné indique un toolset `v145` ; il faut disposer d'un toolset Visual Studio compatible, ou adapter la configuration à celui qui est installé.

### Commandes disponibles

```text
programme --help
programme embed source.bmp destination.bmp "message" [--key "cle"]
programme extract image.bmp [--key "cle"]
```

Exemples depuis la racine du projet (choisir un nom de sortie qui n'existe pas déjà) :

```bat
"x64\Debug\ProjetStéganography.exe" embed "ImageStéga.bmp" "exemple_simple.bmp" "Salut"
"x64\Debug\ProjetStéganography.exe" extract "exemple_simple.bmp"

"x64\Debug\ProjetStéganography.exe" embed "ImageStéga.bmp" "exemple_xor.bmp" "Salut" --key "ABC"
"x64\Debug\ProjetStéganography.exe" extract "exemple_xor.bmp" --key "ABC"
```

Les guillemets sont utiles pour un texte, un chemin ou une clé contenant des espaces. Le programme renvoie `0` si l'opération réussit et `1` en cas d'erreur. Les chemins relatifs sont interprétés à partir du dossier courant du terminal.

Pour l'affichage de certains accents dans le CMD Windows utilisé pendant nos essais, nous avons dû lancer :

```bat
chcp 1252
```

Ce réglage concerne l'affichage de la console, pas les octets cachés dans l'image.

---

## 7. Tests réalisés

Nous avons testé le programme dans Visual Studio et dans le CMD Windows. Voici les résultats observés lors du développement :

| Test | Résultat |
| --- | --- |
| Insertion et extraction du texte `Salut` sans clé | Réussi |
| Insertion et extraction de `Salut` avec la clé `ABC` | Réussi |
| Extraction avec la mauvaise clé `XYZ` | Texte incorrect affiché, sans détection automatique |
| Extraction d'un message chiffré sans clé | Refusée |
| Clé vide avec `--key ""` | Refusée |
| Insertion puis extraction de `Salut à toi` | Réussi après réglage d'affichage CMD |
| Message de 15 octets dans un BMP 8 × 8 | Accepté |
| Message de 16 octets dans ce BMP | Refusé : capacité dépassée |
| Extraction d'une image ne contenant pas de signature `STEG` | Refusée |
| Sauvegarde vers une destination existante | Refusée |
| Chargement d'un fichier inexistant | Refusé |
| Chargement d'un BMP volontairement tronqué à 100 octets | Refusé |
| BMP 5 × 10 pixels avec 1 octet de padding par ligne | Insertion et extraction réussies |
| Comparaison binaire du BMP 5 × 10 avant/après insertion | En-tête et padding inchangés, seules des composantes couleur ont été modifiées |
| Commande `--help` | Aide et exemple affichés |

Les fichiers `ImageStéga.bmp`, `padding.bmp`, `padding_cache.bmp`, `rendu.bmp` et `max.bmp` sont présents dans notre dépôt pour illustrer plusieurs scénarios. Lors de la comparaison avec `fc /b`, les octets de padding du BMP 5 × 10 n'apparaissent pas parmi les modifications.

Nous avons également vérifié que la version refactorisée pouvait relire une image créée avec l'ancienne version.

**À confirmer avant la remise :** compilation depuis un **clone propre**, et vérification croisée formalisée par les deux membres du binôme. Ces points ne sont pas présentés comme déjà validés.

### Reproduire la compilation depuis un clone propre

```bat
git clone --branch Dev https://github.com/TsukiNoKami/Steganography-GTech.git Steganography-GTech-clean
cd Steganography-GTech-clean
```

Ouvrir ensuite la solution, compiler en **Debug | x64**, puis exécuter la commande `--help` et une insertion/extraction simple.

---

## 8. Limites du projet

- Seuls les BMP 24 bits non compressés avec un `BITMAPINFOHEADER` de 40 octets et des dimensions positives sont acceptés.
- Une image trop petite, un fichier tronqué ou une longueur de message incohérente sont refusés.
- Le XOR répété n'assure pas la confidentialité des informations sensibles et ne détecte pas les mauvaises clés.
- L'affichage des accents peut dépendre de la page de codes utilisée dans le CMD Windows.
- Le programme ne prend pas en charge d'autres types d'images (PNG, JPEG, BMP compressés ou 32 bits).

## 9. Sources et outils utilisés

- Cahier des charges du projet de stéganographie G.Tech 1, année 2026–2027.
- [Fiche de révision C++20 et BMP du professeur](https://github.com/GamingCampus-MillieBourgois-26-27/gtech1_steganographie).
- [Wikipédia — format BMP](https://en.wikipedia.org/wiki/BMP_file_format).
- [HexEd.it](https://hexed.it/) pour l'inspection des octets.
- [Projet3 de Sami Bougrini et Nathan Baurin](https://github.com/Sbougrini/Projet3/tree/dev) consulté comme référence de présentation du README et pour comparer l'organisation du projet ; son code n'est pas celui de notre application.

**Assistance IA :** ChatGPT a été utilisé pendant le développement pour des explications, des suggestions et la génération ou la réorganisation de portions de code, ainsi que pour la revue des tests et une première rédaction de ce README. Le cahier des charges interdisant la génération de code par IA, cette utilisation doit être signalée et clarifiée avec l'enseignant avant le rendu.
