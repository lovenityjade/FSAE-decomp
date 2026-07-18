# Décompilation et reconstruction ARM9i

ARM9i ne contient finalement **aucun code propriétaire du jeu**. Les 18 304 octets chargés sont entièrement constitués de sections TwlSDK sélectionnées et de layout; les 20 octets restants du fichier sont le préfixe et la table d'autoload. Il n'y a donc pas de blob de jeu à transcrire en C et aucune raison de copier un binaire SDK dans le dépôt.

La preuve complète est générée dans `build/arm9i/analysis.json`. Elle vérifie les archives externes, les 146 frontières de fonctions, les objets, les hashes des anciens trous et la table LTD.

## Correction du mapping d'exécution

Le fichier ARM9i fait `0x4794` octets. Son premier mot nul est un préfixe de fichier; le payload décrit par la table LTD commence à l'offset fichier `0x4`, pas à `0x0`.

| Région | Offset fichier | Taille | Adresse d'exécution |
|---|---:|---:|---:|
| Préfixe | `0x0000` | `0x4` | non chargé |
| Payload LTDMAIN | `0x0004` | `0x4780` | `0x021870c0` |
| Table LTD autoload | `0x4784` | `0x10` | non chargée |

La conversion correcte est donc :

```text
adresse = 0x021870c0 + offset_fichier - 4
```

Les adresses de la première passe dans `config/sdk-signatures-arm9i.json` étaient quatre octets trop hautes, même si les offsets fichier et les correspondances restaient valides. Le passage Thumb vers ARM se trouve à l'offset fichier `0x664`, adresse réelle `0x02187720`.

La table à `0x4784` contient `0x021870c0`, `0x4780`, `0x021870c0`, `0x34420` : destination, taille initialisée, début des initialisations statiques et taille BSS.

Ce format vient directement du modèle `include/twl/specfiles/ARM9-TS.lcf.template` du SDK. ARM9i n'est pas un second exécutable lié seul : le lien complet ARM9 émet un `Ltdautoload LTDMAIN`, déclaré par `include/twl/specfiles/ARM9-TS.lsf` avec placement `After $(TARGET_NAME)`. Le modèle écrit aussi le mot nul initial et l'enregistrement d'autoload final. L'assembleur local reproduit ces 20 octets de structure pour les tester indépendamment, mais la reconstruction finale doit utiliser le même lien ARM9/LCF.

## Objets complets et ordre

| Fichier | Plage | Archive / objet |
|---|---:|---|
| layout | `0x0000–0x0024` | préfixe et 32 octets de remplissage |
| Thumb | `0x0024–0x004c` | `libna…thumb.a:na_archive.o` |
| Thumb | `0x004c–0x03bc` | `libna…thumb.a:na_rom.o` |
| Thumb | `0x03bc–0x0638` | `libna…thumb.a:na_shared_font.o` |
| Thumb/ARM | `0x0638–0x06c4` | `libsyscall.twl.a:syscall_twl.o` |
| ARM | `0x06c4–0x1f70` | `libfs…a:fs_archive_fatfs` |
| ARM | `0x1f70–0x2548` | `libmi…a:mi_ndma.o` |
| ARM/data | `0x2548–0x34b4` | `libmi…a:mi_sharedwram.o` |
| ARM | `0x34b4–0x34fc` | `libos…a:os_interrupt.o` |
| ARM | `0x34fc–0x3518` | `libos…a:os_arena.o` |
| ARM | `0x3518–0x3564` | `libos…a:os_ownerinfoex.o` |
| ARM | `0x3564–0x35f4` | `libos…a:os_application_j` |
| ARM | `0x35f4–0x394c` | `libspi…a:pm.o` |
| data | `0x394c–0x3954` | `libcard…a:card_utility.o` |
| ARM | `0x3954–0x3b08` | `libcard…a:card_rom.o` |
| data/ARM | `0x3b08–0x4770` | `libcard…a:card_hash.o` |
| layout | `0x4770–0x4784` | 20 octets de remplissage |
| layout | `0x4784–0x4794` | table LTD autoload |

Les sept archives nécessaires et leurs SHA-256 sont épinglées dans `config/build/arm9i.json`. Elles restent sous `TWLSDK_ROOT`.

L'outil public `tools/linker/arm9i_sdk_prepare.py` matérialise cette frontière
sans copier les archives. La commande `inventory` vérifie leurs hashes et la
présence non ambiguë des 15 membres requis, puis écrit uniquement leurs noms,
tailles et SHA-256 sous `build/linker/arm9i-sdk/`. La commande `extract` crée un
jeu adressé par contenu contenant exactement ces membres. Aucun chemin absolu
du SDK, nom de membre non sélectionné ou contenu d'objet n'apparaît dans les
rapports; tous les objets extraits restent sous `build/` et hors du dépôt.

## Les 1 232 octets auparavant non couverts

La première passe ne reconnaissait que les fonctions possédant une ancre unique d'au moins huit octets après masquage des relocations. Les trous ne sont pas du glue de jeu :

| Plage fichier | Taille | Attribution |
|---|---:|---|
| `0x0004–0x0024` | 32 | remplissage initial nul |
| `0x0638–0x06c4` | 140 | cœurs `SVCi_*` de `libsyscall.twl.a`, transition Thumb/ARM et literal pool |
| `0x0f18–0x103c` | 292 | `FSi_FATFS_DeleteFile`, `FSi_FATFS_RenameFile` |
| `0x10dc–0x1200` | 292 | `FSi_FATFS_DeleteDirectory`, `FSi_FATFS_RenameDirectory` |
| `0x13f4–0x145c` | 104 | `FSi_FATFS_CloseFile` |
| `0x1bd0–0x1c38` | 104 | `FSi_FATFS_CloseDirectory` |
| `0x1de0–0x1df8` | 24 | `FSi_SleepProcedure`, `FSi_ShutdownProcedure` |
| `0x253c–0x256c` | 48 | `MI_Card_NDmaCopy` et trois tables de dispatch WRAM |
| `0x25f0–0x2610` | 32 | `MIi_SetWramBankMaster` |
| `0x2688–0x26a8` | 32 | `MIi_SetWramBankEnable` |
| `0x2784–0x27ac` | 40 | `MIi_GetAddress_B`, `MIi_GetAddress_C` |
| `0x394c–0x3954` | 8 | `CARDiDmaUsingNew` (`MI_Card_NDmaCopy`, `MI_StopNDma`) |
| `0x3b08–0x3b48` | 64 | `CARDiHmacKey` de `card_hash.c` |
| `0x4770–0x4784` | 20 | remplissage final nul |

Total : `1 232` octets; octets propriétaires du jeu : `0`.

## Exactitude brute et relocations

Les 146 frontières représentent 18 138 octets de fonctions :

- 37 fonctions, soit 1 710 octets, sont déjà identiques aux sections ELF non liées;
- 109 fonctions, soit 16 428 octets, correspondent après masquage des relocations;
- les 166 autres octets du payload sont 52 octets de remplissage et 114 octets de tables, clés ou literal pools SDK.

Une égalité avec une section ELF externe est une preuve d'identification, pas une sortie reconstruite. Ces 1 710 octets ne sont donc pas crédités comme `matched` par le dashboard. Il faut d'abord lier une vraie image aux adresses finales.

Le seul crédit matching actuel vient de `src/arm9i/layout.S`. Cet assembleur écrit, sans `incbin`, produit les quatre sections de layout appartenant au dépôt : préfixe, deux remplissages et table LTD. Les 72 octets obtenus ont été comparés à la cible et sont identiques.

## Commandes reproductibles

```sh
export TWLSDK_ROOT=/chemin/vers/TwlSDK
python3 tools/linker/arm9i_sdk_prepare.py inventory
python3 tools/linker/arm9i_sdk_prepare.py extract
python3 tools/linker/arm9i_sdk_prepare.py validate
mkdir -p build/arm9i
CCACHE_DISABLE=1 clang --target=arm-none-eabi \
  -c src/arm9i/layout.S -o build/arm9i/layout.o
python3 tools/analysis/arm9i_map.py
python3 tools/progress/progress.py validate
```

Les sorties restent dans `build/arm9i/`, ignoré par Git. Les tests sans SDK externe sont lancés avec :

```sh
python3 -m unittest discover -s tools/analysis/tests -v
```

## Plan de lien vers 100 %

1. Générer le LCF et la response du lien ARM9 depuis `include/twl/specfiles/ARM9-TS.lsf` et `ARM9-TS.lcf.template`, comme le fait `makelcf.TWL.exe`. Le bloc à reconstruire est `Ltdautoload LTDMAIN`, placé après l'image statique ARM9 à `0x021870c0` dans cette ROM.
2. Utiliser les archives épinglées en mode ROM/LTD et sélectionner uniquement les sections atteignables des objets listés ci-dessus. La response doit préserver cet ordre exact; le linker conserve d'abord les sections Thumb de NA et syscall, puis passe en ARM à `0x02187720`.
3. Exécuter `mwldarm` 2.0 build 99 avec l'ABI CodeWarrior 4.0 build 1051 et les drapeaux SDK vérifiés : `-proc arm946e -nothumb -nopic -nopid -interworking -stdlib -map closure -main _start`. La licence privée CodeWarrior reste le prérequis externe pour ce lien; aucun objet SDK n'a besoin d'être recompilé.
4. Extraire le fichier ARM9i émis par le lien complet. Le template doit produire le préfixe, les alignements, le payload LTDMAIN, sa BSS de `0x34420` octets et la table d'autoload; `layout.o` est seulement la preuve locale indépendante de ces quatre zones sérialisées.
5. Passer l'image obtenue à `python3 tools/analysis/arm9i_map.py --candidate chemin/arm9i.bin`. Le fragment matching ne passera à 18 324/18 324 que si cette image est identique octet par octet.

Ce plan réduit ARM9i à un problème de sélection, ordre et relocation d'objets officiels; aucune décompilation de logique de jeu n'y reste.
