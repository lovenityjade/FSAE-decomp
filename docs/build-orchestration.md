# Orchestration publique du build

Le `Makefile` et `tools/build/orchestrate.py` fournissent une façade unique au-dessus des pipelines ROM, cartographie, matching, progression et Windows. Cette couche ne contient aucun dump, binaire SDK, secret SSH ou chemin privé. Elle exige ces valeurs à l’exécution et délègue le travail aux outils spécialisés déjà documentés.

Afficher les commandes disponibles :

```sh
make help
python3 tools/build/orchestrate.py --help
```

Une cible Make accepte les options supplémentaires du dispatcher avec `ARGS`, par exemple :

```sh
make serve ARGS='--port 9000'
```

## Préparation des entrées privées

`diagnose` et `prepare` n’essaient pas de deviner l’emplacement d’un dump. Les trois entrées doivent être fournies par options, environnement ou variables Make :

| Entrée | Option | Environnement | Make |
| --- | --- | --- | --- |
| ROM personnelle | `--rom` | `FSAE_ROM` | `ROM` |
| BIOS7 personnel | `--bios7` | `FSAE_BIOS7` | `BIOS7` |
| Archive TWLTool | `--twltool-zip` | `TWLTOOL_ZIP` | `TWLTOOL` |

Exemple avec des chemins locaux non versionnés :

```sh
make diagnose \
  ROM='/chemin/local/jeu.nds' \
  BIOS7='/chemin/local/bios7.bin' \
  TWLTOOL='/chemin/local/twltool-v1.6.zip'

make prepare \
  ROM='/chemin/local/jeu.nds' \
  BIOS7='/chemin/local/bios7.bin' \
  TWLTOOL='/chemin/local/twltool-v1.6.zip'
```

Les mêmes opérations peuvent être lancées directement :

```sh
python3 tools/build/orchestrate.py diagnose \
  --rom /chemin/local/jeu.nds \
  --bios7 /chemin/local/bios7.bin \
  --twltool-zip /chemin/local/twltool-v1.6.zip
```

Le dispatcher vérifie d’abord que les trois valeurs ont été données et que chaque fichier existe. Le pipeline ROM applique ensuite ses contrôles de taille, hash et identité. Une entrée absente ou incorrecte produit un code non nul; `prepare` ne transforme jamais cet échec en succès.

Options utiles de `prepare` :

- `--dry-run` applique les diagnostics sans écrire;
- `--force` remplace explicitement un ancien `build/rom/`;
- `--output` choisit un autre répertoire généré;
- `--wine` choisit explicitement l’exécutable Wine.

## Cartographie et validation

Après `prepare` :

```sh
make map
make validate
```

`map` exécute `analysis/nds_binary_map.py`, puis appelle automatiquement la même validation stricte que `validate`. La ROM chiffrée originale est facultative et ne sert qu’aux métadonnées de comparaison :

```sh
make map ROM='/chemin/local/jeu.nds'
```

`validate` est entièrement en lecture seule. Il vérifie :

- que `tools/progress/project.v2.json` contient toujours les quatre tailles canoniques;
- que `config/binary-map.json`, `config/nitrofs.json` et `build/rom/manifest.json` décrivent le même SRL déchiffré;
- les identifiants, tailles et SHA-256 des quatre exécutables préparés;
- le nombre, les chemins, tailles et SHA-256 des 29 fichiers NitroFS préparés.

Un fichier absent, altéré ou provenant d’une autre préparation fait échouer la commande. Les chemins peuvent être remplacés avec `--binary-map`, `--nitrofs-map`, `--manifest` et, pour `map`, `--decrypted-rom`.

## Matching

Le matching ARM7/ARM7i requiert un TwlSDK externe :

```sh
make match TWLSDK='/chemin/local/TwlSDK'
```

ou :

```sh
TWLSDK_ROOT='/chemin/local/TwlSDK' \
  python3 tools/build/orchestrate.py match
```

Le code retour du comparateur n’est jamais accepté seul. Après son succès, l’orchestrateur relit chaque configuration et :

1. recalcule taille et SHA-256 de la référence SDK, de la cible préparée et de l’artefact produit;
2. exige une preuve `binary-match-proof` dont la comparaison est exactement byte-for-byte, sans octet divergent;
3. exige que référence, cible et normalisation soient marquées vérifiées et correspondent aux hashes configurés;
4. exige un fragment de progression v2 `track: matching`, catégorie `sdk`, créditant exactement la taille canonique;
5. exécute enfin `progress-validate`.

Un mismatch du comparateur conserve son code non nul. Un comparateur qui retournerait zéro sans preuve, avec une preuve périmée ou avec un fragment de progression incomplet est également un échec. Aucun chemin de l’orchestrateur ne publie lui-même de faux octets matched.

Des configurations particulières peuvent être sélectionnées avec plusieurs `--config`, mais seules les preuves de ces configurations sont alors validées.

## Progression, serveur et tests

```sh
make progress-validate
make serve
make test
```

`progress-validate` contrôle le schéma et tous les fragments. `serve` écoute par défaut uniquement sur `127.0.0.1:8765`. `test` lance successivement les suites publiques ROM, matching, progression, Windows, outils de décompilation et orchestration, puis la validation de progression. Il ne transforme jamais l’échec d’une suite en succès.

## Hôte Windows

L’orchestrateur n’a aucun alias SSH ni chemin Windows par défaut. Ils doivent tous venir de l’utilisateur. Les valeurs reconnues sont :

| Valeur | Option | Environnement | Make |
| --- | --- | --- | --- |
| Alias SSH | `--host` | `FSAE_WINDOWS_HOST` | `WINDOWS_HOST` |
| Workspace absolu | `--remote-workspace` | `FSAE_WINDOWS_WORKSPACE` | `WINDOWS_WORKSPACE` |
| Racine toolchain privée | `--toolchain-root` | `FSAE_WINDOWS_TOOLCHAIN_ROOT` | `WINDOWS_TOOLCHAIN` |
| Script d’environnement privé | `--environment-script` | `FSAE_WINDOWS_ENVIRONMENT_SCRIPT` | `WINDOWS_ENVIRONMENT` |
| Commande de build | `--command` | `FSAE_WINDOWS_BUILD_COMMAND` | `WINDOWS_COMMAND` |

Pour les chemins Windows contenant des espaces ou des antislashs, les variables d’environnement évitent les niveaux de quoting Make/shell :

```sh
export FSAE_WINDOWS_HOST='mon-alias-ssh'
export FSAE_WINDOWS_WORKSPACE='X:\Projet\workspace'
export FSAE_WINDOWS_TOOLCHAIN_ROOT='X:\Projet\toolchains'
export FSAE_WINDOWS_ENVIRONMENT_SCRIPT='X:\Projet\private\environment.ps1'

make windows-sync ARGS='--dry-run'
make windows-check

export FSAE_WINDOWS_BUILD_COMMAND='python tools\windows\smoke_build.py'
make windows-build
```

`windows-sync` préserve par défaut les fichiers distants et les emplacements privés. `--delete-stale` reste une option explicite. `windows-check` ne construit rien. `windows-build` exige une commande non vide et le helper Windows revérifie les compilateurs avant de l’exécuter. Les détails de transport et d’exclusion restent documentés dans `docs/windows-toolchain.md`.

## Codes de sortie

- `0` signifie que la commande déléguée et toutes les validations postérieures ont réussi;
- `1` est conservé pour un mismatch ou un test délégué ayant échoué avec ce code;
- `2` indique une entrée absente, une configuration invalide, une preuve incohérente ou une erreur d’orchestration.

Les commandes sont affichées avant exécution pour rendre le workflow reproductible. Aucun contenu propriétaire n’est copié dans les fichiers publics par cette couche.
