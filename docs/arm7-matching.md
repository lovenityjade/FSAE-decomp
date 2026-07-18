# Matching ARM7 et ARM7i

Les deux processeurs ARM7 proviennent du composant officiel `racoon` de l'arbre TwlSDK 5.5 patch 4 préparé par `tools/sdk/prepare_patch4.py`. Aucun binaire du SDK ni de la ROM n'est conservé dans le dépôt : les entrées viennent d'un SDK externe et de `build/rom/`, puis les résultats et preuves sont écrits dans `build/match/` (ignoré par Git).

## Reproduire la preuve

`TWLSDK_ROOT` doit désigner le répertoire `TwlSDK` patch 4 validé qui contient `components/racoon/ARM7-TS.LTD/Rom/`.

```sh
export TWLSDK_ROOT=/chemin/vers/TwlSDK
python3 tools/match/arm7_match.py
python3 tools/progress/progress.py validate
```

Le comparateur vérifie d'abord la taille et le SHA-256 de chaque entrée officielle et de chaque cible. Il produit ensuite :

- `build/match/arm7.bin`, `arm7.proof.json` et `arm7.progress.json`;
- `build/match/arm7i.bin`, `arm7i.proof.json` et `arm7i.progress.json`.

Les fragments `*.progress.json` utilisent la piste `matching`. Ils déclarent tous les octets comme `matched` uniquement lorsque le fichier produit est identique à la cible octet par octet; sinon, ils en déclarent zéro.

## Pourquoi la normalisation ARM7 est limitée et justifiée

Le `racoon.TWL.FLX.sbin` patch 4 reconstruit a exactement une différence structurelle avec l'ARM7 de la ROM :

1. Le fichier SDK se termine par 16 octets qui ne font pas partie du segment ARM7 embarqué. Le template officiel `include/twl/specfiles/ARM7-TS.lcf.template`, section `binary.STATIC_FOOTER`, définit ces quatre mots : magie `0xdec00621` (`NITRO CODE`), offset de `_start_ModuleParams`, digest nul et offset de `_start_LtdModuleParams`. Pour ce composant, ils valent respectivement `0xdec00621`, `0x544`, `0` et `0x52c`. Le comparateur vérifie les 16 octets et chacun des champs avant de retirer le footer.

Aucune autre transformation n'est acceptée. Le marqueur patch 4 est déjà présent dans l'entrée externe, dont le SHA-256 doit être `dd6ec062…6af393`. Après retrait du footer, le SHA-256 obtenu doit être `a96394d986dced7cbe6f3b3d2f82464b3970b82f336e3d26500419dbe380de1c`, puis l'égalité binaire complète avec `build/rom/sections/arm7.bin` est vérifiée.

ARM7i ne demande aucune normalisation : `racoon.TWL.LTD.sbin` et `build/rom/sections/arm7i.bin` doivent tous deux avoir le SHA-256 `3703c27125294aa754fb4bc64f6f78e6cac93ae0b4eb4d72cafbcc591aaaa3ef`.

## Tests synthétiques

```sh
python3 -m unittest discover -s tools/match/tests -v
```

Les fixtures couvrent l'identité directe, le footer structuré, la normalisation historique explicite d'un marqueur, le refus d'un footer ou d'un marqueur inattendu, le refus d'un hash officiel incorrect et la règle stricte qui publie zéro octet en cas de mismatch.
