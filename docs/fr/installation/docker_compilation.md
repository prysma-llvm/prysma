# Compilation et benchmarks (Docker éphémère)

Ce conteneur permet de compiler Prysma et d'exécuter la suite de tests de performance de manière isolée sur le serveur.

L'exécution de ce conteneur est entièrement automatisée par le workflow GitHub Actions décrit dans [github_action.md](github_action.md). Les commandes ci-dessous sont documentées à titre de référence pour l'administration et le débogage sur le serveur.

## 1. Construction de l'image

Le build de l'image doit être lancé depuis le dossier `~/prysma` sur le serveur :

```bash
docker build -t prysma-compiler -f Dockerfile .
```

## 2. Exécution manuelle (débogage sur le serveur)

La commande doit être lancée depuis le dossier `~/prysma` (contenant le sous-dossier de dépôt `prysma/`) :

```bash
docker run --rm --privileged --cpuset-cpus="1-3" -v "$(pwd)/prysma":/workspace prysma-compiler
```

* **`--rm`** : Supprime le conteneur à la fin du processus.
* **`--privileged`** : Accès aux compteurs matériels du processeur hôte (PMU).
* **`--cpuset-cpus="1-3"`** : Assigne l'exécution aux cœurs stabilisés par le Mode Laboratoire (le cœur 0 reste libre pour le système).
* **`-v "$(pwd)/prysma":/workspace`** : Monte le sous-dossier `prysma/` dans le répertoire `/workspace` du conteneur pour que `tests/run_perf_tests.py` s'exécute correctement.

## 3. Enregistrement des résultats

Une fois le rapport `perf_run_data.json` généré par le conteneur, le script d'enregistrement est lancé directement depuis l'hôte :

```bash
python3 prysma/tests/save_perf_results.py
```

## 4. Compteurs matériels (PMU) et Déverrouillage Local

Le conteneur est toujours exécuté avec l'option `--privileged` pour accéder aux compteurs matériels du processeur hôte (cycles, ratés de cache L1/L2/L3, branch misses). Le paramètre `--cpuset-cpus="1-3"` assigne l'exécution aux cœurs stabilisés par le Mode Laboratoire.

En production, ces paramètres sont appliqués automatiquement par le workflow GitHub Actions et le Mode Laboratoire gère lui-même le déverrouillage temporaire du noyau sur le serveur.

### Déverrouillage manuel pour développement/débogage (sans conteneur)

Si vous souhaitez exécuter le script de benchmark directement sur le serveur hors du conteneur Docker, le noyau Linux bloquera par défaut l'accès aux compteurs matériels. 

Pour déverrouiller temporairement l'accès :
```bash
sudo sysctl -w kernel.perf_event_paranoid=-1
sudo sysctl -w kernel.kptr_restrict=0
```

Une fois cette commande exécutée, vous pouvez lancer les tests directement sur l'hôte depuis le sous-dossier :
```bash
cd ~/prysma/prysma/tests
python3 run_perf_tests.py
```
