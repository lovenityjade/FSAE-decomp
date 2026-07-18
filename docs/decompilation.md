# Décompilation ARM9

La décompilation active porte sur l'ARM9. ARM7 et ARM7i sont déjà reproduits
exactement par le composant externe `racoon`; ARM9i est entièrement classé
comme code TwlSDK et layout. Le link final reste volontairement reporté.

L'ELF d'analyse mappe l'image statique et les autoloads à leurs vraies adresses
d'exécution. Son projet Ghidra se trouve sous `build/ghidra/`, qui reste ignoré
par Git. L'export brut se régénère avec :

```sh
tools/decomp/run_ghidra_headless.sh export arm9 0x02004000 0x0212727f
python3 tools/decomp/catalog.py
python3 tools/decomp/rtti_catalog.py
```

Le catalogue produit sous `build/decomp/arm9/` sépare trois classes :

- `sdk` : frontière de fonction prouvée indépendamment dans une archive SDK;
- `game_candidate` : pseudocode non-SDK à réécrire et nommer;
- `switch_artifact` : cible `caseD_` créée par Ghidra, pas une fonction source.

Les fichiers `modules/game_raw_*.c` sont des shards de travail. Ils ne sont ni
compilables, ni du code matching, et ne donnent aucun crédit de reconstruction.
Leur rôle est de rendre chaque fonction du jeu consultable pendant la reprise
sémantique. `game_candidates.csv` est la file de travail déterministe.

`rtti_catalog.py` récupère aussi les objets RTTI Itanium et leurs tables
virtuelles. Il associe les slots aux fonctions Ghidra sans inventer le nom des
méthodes. Ces relations de classes servent ensuite au renommage sémantique.

Les reprises publiques se trouvent sous `src/arm9/`. Elles couvrent maintenant
des unités jeu indépendantes (`game/`) et plusieurs familles RTTI `ntmv::m2d` :

- `Page`, `Pane`, `Picture`, `Window` et `TextBox`;
- `ResourceAccessor` et `ArcResourceAccessor`;
- `UIElement`, `UIStatic`, `UIPanel` et `ObjButton`;
- `TexelPicker`, `DirectTexelPicker` et `Palette256TexelPicker`.

Chaque module conserve les adresses ARM9 dans ses commentaires, documente les
noms encore descriptifs/provisoires et possède un contrôle C strict ou un
harness hôte. Les unités `game/` couvrent notamment DataManager, DatalinkReader,
le formatage/log interne, le chemin de terminaison et plusieurs routines HUD.

Les noms revus sont appliqués au projet Ghidra par un registre public gardé :

```sh
python3 tools/decomp/apply_semantic_symbols.py validate
python3 tools/decomp/apply_semantic_symbols.py apply --dry-run
python3 tools/decomp/apply_semantic_symbols.py apply
```

Le wrapper vérifie les hashes du binaire/ELF, l'ancien nom attendu et la
présence de la fonction. Une création d'entrée n'est permise que par une garde
explicite `create_if_missing`, utilisée pour les adresses prouvées par une
vtable ou un corps d'instructions mais initialement laissées comme labels par
Ghidra. Un second dry-run doit toujours être entièrement idempotent.

`semantic_progress.py` recalcule la preuve de progression à partir des adresses
standalone présentes dans les sources et des tailles du catalogue Ghidra :

```sh
python3 tools/decomp/semantic_progress.py
python3 tools/progress/progress.py validate
```

Pendant cette phase, le tableau de bord présente la décompilation/analyse comme
mesure principale. Le byte-match reste visible séparément et ne bougera que
lorsqu'un résultat de reconstruction byte-for-byte sera établi durant la phase
de link finale.
