# Tableau de bord de progression

Le tableau de bord local recalcule la progression de la décompilation à partir de petits fragments JSON indépendants. Il ne lit pas la ROM, n’embarque aucune donnée du jeu et ne rend accessible par HTTP que `dashboard/` et l’API calculée.

## Démarrage

Depuis la racine du projet :

```sh
python3 tools/progress/progress.py validate
python3 tools/progress/progress.py serve
```

Ouvrir ensuite `http://127.0.0.1:8765`. Le navigateur interroge `/api/progress` toutes les cinq secondes. Chaque requête relit les entrées et recalcule les totaux; il n’est donc pas nécessaire de redémarrer le serveur lorsqu’un agent publie une mise à jour. L’API et les assets exposent aussi la même version de dashboard. Si le JavaScript chargé ne correspond plus, le client purge les anciens caches et service workers puis se recharge automatiquement sur l’asset courant.

Un port différent peut être choisi avec `--port`. Le serveur reste volontairement attaché à `127.0.0.1`; utiliser `--host` seulement si une exposition réseau est réellement voulue.

## Modèle de calcul

Le format canonique est décrit par `tools/progress/schema-v2.json`. La configuration versionnée `tools/progress/project.v2.json` déclare :

- les sections exécutables `arm9`, `arm7`, `arm9i` et `arm7i`, qui alimentent le total;
- les catégories transversales optionnelles `sdk`, `game` et `data`, qui apparaissent dès qu’elles ont des métriques et ne sont pas recomptées dans le total;
- les tailles canoniques vérifiées, les workers attendus et les motifs de découverte des fragments.

La baseline byte-matching est fixe :

| Section | Taille |
| --- | ---: |
| ARM9 | `0x123600` — 1 193 472 octets |
| ARM7 | `0x25860` — 153 696 octets |
| ARM9i | `0x4794` — 18 324 octets |
| ARM7i | `0x49710` — 300 816 octets |
| **Total** | **1 666 308 octets** |

Le pourcentage global de décompilation est donc toujours `octets byte-matched / 1 666 308`. Il n’existe plus de moyenne par section ni de fallback pouvant surpondérer un petit binaire. Les catégories transversales sont exclues du rollup puisqu’elles classent les mêmes octets.

### Deux pistes strictement séparées

La piste `matching` représente uniquement une correspondance byte-for-byte vérifiée. Un octet peut y être crédité lorsqu’il provient :

- d’un artefact reconstruit puis comparé au binaire cible;
- d’un composant ou d’une bibliothèque officielle SDK comparé byte-for-byte, avec section, plage et provenance explicites, même si sa source n’est pas encore versionnée dans le dépôt.

Une extraction, un déchiffrement, une frontière de fonction, un symbole, une signature, une classification SDK/jeu, une cartographie NitroFS ou un désassemblage ne comptent jamais comme `matched` à eux seuls.

La piste `analysis` mesure séparément ce travail préparatoire avec des compteurs `covered`. Elle peut atteindre 100 % sans modifier le pourcentage de décompilation. Une preuve v1 historique sans piste explicite est volontairement migrée en mémoire vers `analysis`; elle doit être republiée explicitement en v2 pour créditer du byte-matching.

La progression d’une piste utilise les octets lorsqu’un total existe, sinon les fonctions, sinon les unités. Un ratio inférieur à 100 % n’est jamais arrondi visuellement à 100 %.

Les entrées se trouvent normalement dans :

- `tools/progress/evidence/*.json` pour les unités de travail disjointes;
- `build/**/*.progress.json` pour les sidecars produits automatiquement par une future chaîne de build;
- `tools/progress/workers/*.json` pour les heartbeats;
- `tools/progress/changes/*.json` pour le journal d’activité.

Chaque identifiant de preuve doit être unique dans tous les motifs découverts. Une entrée invalide ou dupliquée est ignorée, signalée dans `issues` et affichée dans le tableau de bord. Les écritures produites par l’outil sont atomiques afin qu’un rafraîchissement HTTP ne voie jamais un fichier partiel.

Les cibles définies dans la configuration sont des dénominateurs, jamais des preuves. Les fragments doivent représenter des ensembles de fonctions/octets disjoints : les compteurs de plusieurs fragments sont additionnés.

## Mise à jour par les agents

Un agent rafraîchit son état avec :

```sh
python3 tools/progress/progress.py set-worker \
  --id binary-map \
  --label "Binary map" \
  --status working \
  --section arm9 \
  --task "Identification des frontières de fonctions"
```

Les états permis sont `working`, `idle`, `blocked`, `done` et `offline`. Un worker `working` ou `idle` dont le heartbeat date de plus de 180 secondes est présenté hors ligne. L'activité d'un processus ou d'un agent n'est pas détectée automatiquement : `updated_at` dans le fragment JSON est l'unique source de présence.

Lorsqu'une tâche ou un statut change, republier `set-worker` avec les nouvelles valeurs. Pendant une tâche longue inchangée, rafraîchir uniquement l'horodatage avant l'expiration avec :

```sh
python3 tools/progress/progress.py heartbeat-worker --id binary-map
```

Cette commande valide le worker existant et réécrit atomiquement seulement `updated_at`; elle ne modifie ni la tâche, ni le statut, ni les métriques de progression.

Une unité reconstruite ou une plage SDK comparée byte-for-byte se publie explicitement sur `matching` :

```sh
python3 tools/progress/progress.py set-evidence \
  --id arm9-example-unit \
  --track matching \
  --section arm9 \
  --category game \
  --worker binary-map \
  --units-matched 1 --units-total 1 \
  --functions-matched 7 --functions-total 10 \
  --bytes-matched 384 --bytes-total 640 \
  --summary "Unité ARM9 reconstruite et comparée byte-for-byte"
```

Une couverture d’analyse utilise `covered`, jamais `matched` :

```sh
python3 tools/progress/progress.py set-evidence \
  --id arm9-sdk-signatures \
  --track analysis \
  --section arm9 \
  --category sdk \
  --worker binary-map \
  --functions-covered 942 --functions-total 942 \
  --bytes-covered 111094 --bytes-total 1193472 \
  --summary "Frontières SDK ARM9 identifiées"
```

Répéter la commande avec le même identifiant remplace atomiquement ce fragment. Les compteurs `matched` ou `covered` ne peuvent pas dépasser leurs totaux et les deux vocabulaires sont mutuellement exclusifs. La catégorie est facultative et peut être `sdk`, `game` ou `data`.

Un événement qui ne modifie pas directement les compteurs peut être ajouté au journal :

```sh
python3 tools/progress/progress.py add-change \
  --section arm7 \
  --worker rom-pipeline \
  --summary "Configuration ARM7 vérifiée"
```

Pour inspecter exactement ce que l’API exposera :

```sh
python3 tools/progress/progress.py collect --pretty
```

## Validation et tests

La validation contrôle la configuration, tous les fragments découverts, l’unicité des identifiants et la présence des ressources du site :

```sh
python3 tools/progress/progress.py validate
```

La suite standard couvre la baseline canonique, l’isolation matching/analyse, la migration sûre des preuves v1, les agrégations, les erreurs de compteurs, les doublons, les heartbeats et le périmètre HTTP :

```sh
python3 -m unittest discover -s tools/progress/tests -v
```

Une vérification HTTP manuelle peut se faire pendant que le serveur tourne :

```sh
curl http://127.0.0.1:8765/healthz
curl http://127.0.0.1:8765/api/progress
```

`/healthz` renvoie `ok` lorsque toutes les entrées sont valides, `degraded` si certaines ont été ignorées, et une erreur HTTP 500 si la configuration ne peut pas être chargée.
