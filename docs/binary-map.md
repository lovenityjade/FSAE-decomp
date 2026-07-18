# Cartographie binaire initiale

Cette cartographie décrit l’édition européenne `KQ9V` de *The Legend of Zelda:
Four Swords Anniversary Edition*. Elle repose sur la copie d’analyse entièrement
déchiffrée `/tmp/fsae-analysis-decrypted.nds`; aucun exécutable, fichier NitroFS,
BIOS ou matériel de clé n’est ajouté au projet.

Les résultats reproductibles sont dans :

- `config/binary-map.json` : header, images exécutables, chargements, autoloads,
  digests, modcrypt et disposition ROM;
- `config/nitrofs.json` : FNT/FAT, noms, offsets, tailles et SHA-256 des 29 fichiers;
- `config/sdk-identification.json` : versions et identifications SDK;
- `config/sdk-signatures-arm9.json` et `config/sdk-signatures-arm9i.json` :
  correspondances de fonctions masquées aux relocations;
- `config/sdk-symbols-arm7.json` : 2 523 symboles de fonctions issus du TEF
  officiel `racoon`, projetés sur ARM7 et ARM7i.

## Identité et état cryptographique

| Propriété | Valeur |
|---|---:|
| Titre interne | `ZELDA 4SWORD` |
| Code / fabricant | `KQ9V` / `01` |
| Type | DSi/TWL only, unit code `3` |
| Révision | `0` |
| Taille du header | `0x4000` |
| Taille réelle | `0xE29000` (14 848 000 octets) |
| SHA-256 de la ROM originale | `3a880dce73ace38f923eb2c3f3e497ca00749c7349ab14c99ad88710c45a8be8` |
| SHA-256 de la copie d’analyse | `070edd820627d25749c1d24817c36fcb08a082e87cd3f4e0ad39ac41af84ded8` |

Le déchiffrement est suffisant pour l’analyse des quatre images :

- l’ARM9 possède une secure area de `0x4000` octets, mais la comparaison montre
  que seul son préfixe `0x0000–0x07FF` était Blowfish-chiffré. L’entrée réelle
  se trouve justement après ce préfixe, à l’offset binaire `0x800`;
- l’ARM7 est déjà en clair dans la ROM originale;
- ARM9i correspond à modcrypt area 0 (`0xDDB000`, taille AES alignée `0x47A0`);
- ARM7i correspond à modcrypt area 1 (`0xDDF800`, `0x49710`).

La zone modcrypt ARM9i couvre 12 octets de remplissage après les `0x4794`
octets déclarés de l’image. Cette différence doit être conservée lors d’une
reconstruction de conteneur.

## Images exécutables

| Image | ROM | Taille | Chargement initial | Entrée / mode confirmé | SHA-256 déchiffré |
|---|---:|---:|---:|---|---|
| ARM9 | `0x004000` | `0x123600` | `0x02004000` | `0x02004800`, ARM | `8bee49ee…27300aa` |
| ARM7 | `0x127600` | `0x025860` | `0x02380000` | `0x02380000`, ARM | `a96394d9…380de1c` |
| ARM9i | `0xDDB000` | `0x004794` | staging `0x02400000` | Thumb au début | `ab03826f…a58f6f` |
| ARM7i | `0xDDF800` | `0x049710` | staging `0x02E80000` | ARM au début | `3703c271…aaa3ef` |

Les adresses ARM9i/ARM7i du header sont des zones de staging. Leurs tables
`LTDAUTOLOAD_INFO` en fin d’image décrivent les destinations d’exécution
réelles; traiter toute l’image comme un bloc linéaire aux adresses de staging
donnerait une cartographie erronée.

### ARM9

Le bloc de paramètres de module commence à l’offset `0xFE4` :

- statique initialisé : `0x02004000–0x02127280`;
- BSS statique : `0x02127280–0x021870C0`;
- table d’autoload : `0x021275E0–0x02127600`, à la fin du fichier;
- autoload ITCM : `0x01FF8000–0x01FF82C0`, `0x2C0` octets;
- autoload ARM7 WRAM vu depuis ARM9 : `0x02FE0000–0x02FE00A0`, suivi de
  `0x960` octets de BSS.

L’entrée et presque toutes les signatures SDK trouvées sont ARM. Deux îlots
Thumb sont prouvés par les objets SDK, notamment `FSi_OverrideRomArchive` à
`0x02005470`. Il faut donc conserver l’interworking ARM/Thumb au lieu de fixer
un mode unique pour toute l’image.

Le scan initial de 104 archives Rom TwlSDK/TwlSystem a examiné 9 731 fonctions.
Il trouve 1 021 candidats de nom sur 942 frontières uniques et couvre 111 094
octets. Les deux grands amas les plus nets se situent aux offsets binaires
`0x009EDC–0x00FA38` et `0x010034–0x01CFBC`. D’autres blocs officiels très nets
sont :

- CARD : `0x09F798–0x0A0888`;
- NNS Sound : `0x0BB184–0x0BD0B8`;
- NNS FND/G2D/Sound : `0x0BD268–0x0C30AC` avec quelques coupures;
- IRQ/autoload de fin d’image : `0x1232A0–0x123528`.

Une correspondance est unique après masquage des octets relocalisés. Les
petites fonctions identiques peuvent néanmoins avoir plusieurs noms possibles;
le JSON les conserve avec `boundary_candidate_names`. Les correspondances
isolées très courtes restent des hypothèses, alors que les amas contigus ont une
forte confiance.

### ARM9i

La table LTD décrit une seule destination :

- données initialisées `0x021870C0–0x0218B840` (`0x4780` octets);
- BSS jusqu’à `0x021BFC60` (`0x34420` octets);
- quatre octets d’alignement, puis une entrée de table de 16 octets.

Le code commence en Thumb à l’offset `0x20`. L’inspection des instructions et
les signatures officielles montrent un passage Thumb → ARM à l’offset `0x664`,
soit à l’adresse finale `0x02187724`. Les bibliothèques `NA`, `FS`, `FATFS`,
`MI`, `CARD`, `SPI`, `OS` et quelques fonctions NNS sont déjà reconnues.

Le scan couvre 17 072 des 18 304 octets initialisés (`93,27 %`) sur 125
frontières uniques. ARM9i est donc presque entièrement remplaçable par des
bibliothèques officielles dès la première passe.

### ARM7 et ARM7i : composant officiel racoon

L’identification du composant ARM7 est exacte : `racoon`, variante
`ARM7-TS.LTD/Rom` compilée en ARM.

- L’ARM7i déchiffré est identique octet pour octet au
  `racoon.TWL.LTD.sbin` fourni dans TwlSDK 5.5 patch 5, SHA-256 commun
  `3703c271…aaa3ef`.
- L’ARM7 correspond au préfixe du `racoon.TWL.FLX.sbin` patch 5 sur 153 695
  de ses 153 696 octets. L’unique différence, à `0x55C`, est le numéro de
  patch SDK `4` dans la cible contre `5` dans la référence. La référence a en
  plus un footer SDK de 16 octets.

Le `racoon.tef` officiel contient les symboles et frontières exactes : 973
fonctions sont projetées sur ARM7 et 1 550 sur ARM7i. Les quelques symboles
Thumb sont des routines interworking explicites; le composant est autrement ARM.

Disposition ARM7 :

- bootstrap statique `0x02380000–0x023805B4`;
- autoload initialisé `0x037C0000–0x037E529C`;
- BSS jusqu’à `0x03800F40`.

Disposition ARM7i :

- RSVWRAM initialisé `0x03040000–0x03043F50`, BSS jusqu’à `0x03047A98`;
- LTDMAIN initialisé `0x02F88000–0x02FCD79C`, BSS jusqu’à `0x02FE8210`.

Le mot du header à `0x1D4` vaut `0x03800F40`, exactement la fin du BSS ARM7.
Le header `CARDRomHeaderTWL` du SDK 2009 nomme encore ce champ « reserved »;
sa sémantique tardive exacte doit donc rester marquée inconnue, même si sa
relation avec la limite d’arène ARM7 est prouvée.

## Version SDK et bibliothèques

ARM9 et ARM7 contiennent tous deux le mot de version `0x05057534` suivi des
magics `0xDEC00621` / `0x2106C0DE`. Le décodage officiel de `showversion`
donne **TwlSDK 5.5 release 4**. Il s’agit donc du patch 4, pas du patch 5 fourni
localement.

Le patch 5 reste une référence très proche : tout ARM7i est identique et tout
ARM7 hors marqueur de version l’est aussi. Pour ARM9, les bibliothèques du SDK
de base produisent déjà de très nombreuses correspondances exactes. La présence
de TwlSystem est confirmée; la référence locale 2.2.0 fournit 259 frontières NNS
uniques couvrant 30 804 octets. La révision précise de TwlSystem n’a toutefois
pas encore de marqueur interne équivalent au mot TwlSDK.

Les familles déjà rencontrées comprennent `OS`, `MI`, `FS`, `FATFS`, `GX`,
`CARD`, `SPI`, `RTC`, `WM`, `SND`, `NNS FND`, `NNS G2D` et `NNS SND`.

## NitroFS

La FNT est à `0x14D000` (`0x1B5`), la FAT à `0x14D200` (`0xE8`). Il n’y a
qu’un répertoire, la racine, et exactement 29 fichiers. Les payloads occupent
`0x14F800–0xD930DC` avec des alignements entre fichiers.

| IDs | Groupe | Taille cumulée |
|---|---|---:|
| 0–1 | messages `*.kmsg` | 576 992 |
| 2 | police NFTR | 4 104 |
| 3–4 | `knife_sound` | 1 574 144 |
| 5–6 | ressources/manuels BLZ/NARC | 412 448 |
| 7 | `pat.bin` | 76 024 |
| 8–13 | `subtask*.cmp` | 426 720 |
| 14–21 | huit banques `*.wave` | 6 610 524 |
| 22–27 | `zeldat*.bin` | 2 087 264 |
| 28 | `zelmap.bin` | 1 083 100 |

Le fichier détaillé donne pour chaque ID son chemin, sa plage ROM et son hash.
Les noms `m2dres_narc.blz` et `manpages_narc_eu.blz`, ainsi que les chemins
internes `M2M:/` et `M2U:/` dans ARM9, confirment qu’une partie importante de
l’interface/manuelle repose sur la bibliothèque interne `nt::mv::m2d`.

## Overlays, digests et trous

Les tables d’overlays ARM9 et ARM7 ont toutes deux une taille nulle. Il n’y a
donc aucun overlay NTR à extraire ou reconstruire.

| Région | Début | Taille | Fin |
|---|---:|---:|---:|
| Digest NTR couvert | `0x004000` | `0xD8F400` | `0xD93400` |
| Table SHA-1 secteurs | `0xD93400` | `0x45600` | `0xDD8A00` |
| Table SHA-1 blocs | `0xDD8A00` | `0x22B0` | `0xDDACB0` |
| Digest TWL couvert | `0xDDB000` | `0x4E000` | `0xE29000` |

Le secteur de digest vaut `0x400`; un bloc contient 32 secteurs. Les espaces
entre FAT/banner/fichiers, entre les tables de digest et ARM9i, et entre les
deux images modcrypt sont du padding structurel et ne doivent pas être absorbés
dans les images exécutables.

## Frontières de code propriétaire connues

Deux noms de fichiers source subsistent dans ARM9 :

- `DataManager.cpp` aux offsets `0x116B34` et `0x116B4C`;
- `DatalinkReader.cpp` aux offsets `0x120A94` et `0x120AB4`.

La fin de l’image contient aussi les classes manglées `nt::mv::m2d`, les chemins
de ressources `M2U:/data/...`, `M2M:/arc`, et les chaînes des API `WM_*`.
`m2d` n’existe ni dans TwlSDK 5.5 ni dans TwlSystem 2.2.0 fourni; elle demeure
donc une bibliothèque Nintendo interne à cartographier comme code propriétaire.

## Reproduction et validation

Depuis la racine du projet :

```sh
python3 analysis/nds_binary_map.py /tmp/fsae-analysis-decrypted.nds \
  --encrypted-rom "Zelda-_Four_Swords-Anniversary_Edition-Nintendo (EUR).nds"
python3 -m json.tool config/binary-map.json >/dev/null
python3 -m json.tool config/nitrofs.json >/dev/null
```

La projection ARM7 requiert le TEF officiel extrait localement :

```sh
python3 analysis/map_arm7_tef_symbols.py /chemin/vers/racoon.tef
```

Le scan ARM9 accepte directement les archives `.a` officielles :

```sh
python3 analysis/scan_sdk_libraries.py /tmp/fsae-arm9-analysis.bin \
  /chemin/vers/TwlSDK/lib/ARM9-TS/Rom/*.TWL.LTD.a \
  /chemin/vers/TwlSDK/lib/ARM9-TS/Rom/*.TWL.LTD.thumb.a \
  --image-id arm9 --load-address 0x02004000 \
  --binary-map config/binary-map.json \
  --output config/sdk-signatures-arm9.json
```

Les scripts n’emploient que la bibliothèque standard Python. Ils refusent les
FNT/FAT invalides, les images hors limites, les tables d’autoload incohérentes
et les ELF non ARM. Tous les manifests JSON ont été validés avec
`python3 -m json.tool`.
