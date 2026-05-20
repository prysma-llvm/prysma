# Configuration du Mode Laboratoire (Isolation OptiPlex)

Ce document décrit comment déployer, construire et exécuter le conteneur de contrôle pour activer et désactiver le Mode Laboratoire sur le serveur OptiPlex.

Ce mode permet d'éteindre les services d'arrière-plan (les conteneurs Docker applicatifs et le Moniteur de surveillance) tout en maintenant le tunnel Cloudflare et le service SSH actifs pour préserver la connexion à distance.

## 1. Structure des fichiers

Les sources du conteneur se trouvent dans le dépôt local :
* Fichier exécutable : `docker/server/lab_mode/lab_mode.sh`
* Fichier de build : `docker/server/lab_mode/Dockerfile`

## 2. Déploiement sur le serveur

Exécutez les commandes suivantes pour copier les fichiers de configuration dans le dossier d'exécution sur le serveur :

```bash
mkdir -p ~/lab-mode
cp docker/server/lab_mode/Dockerfile ~/lab-mode/Dockerfile
cp docker/server/lab_mode/lab_mode.sh ~/lab-mode/lab_mode.sh
```

## 3. Construction de l'image Docker

Rendez-vous dans le répertoire cible et construisez l'image Docker :

```bash
cd ~/lab-mode
docker build -t optiplex-lab-mode .
```

## 4. Utilisation

Pour contrôler les services `systemd` de l'hôte, le conteneur utilise l'outil `nsenter` afin d'exécuter les commandes directement dans le contexte du système hôte. Le conteneur doit être exécuté avec les privilèges d'accès au namespace PID hôte (`--privileged --pid=host`).

### Activer le Mode Laboratoire (Calme et isolation)
Cette commande arrête le démon de surveillance et enregistre dynamiquement tous les conteneurs applicatifs en cours d'exécution pour les mettre en pause, isolant ainsi la machine :

```bash
docker run --rm --privileged --pid=host \
  -v /var/run/docker.sock:/var/run/docker.sock \
  optiplex-lab-mode enable
```

### Désactiver le Mode Laboratoire (Restauration du serveur)
Cette commande relance le démon de surveillance et restaure exactement la liste des conteneurs qui ont été suspendus lors de l'activation :

```bash
docker run --rm --privileged --pid=host \
  -v /var/run/docker.sock:/var/run/docker.sock \
  optiplex-lab-mode disable
```

## 5. Stabilisation automatique du système

Le script d'activation du Mode Laboratoire (`lab_mode.sh`) configure automatiquement l'hôte pour réduire la gigue et la variance des mesures.

Lors de l'activation (`enable`), le conteneur applique les règles suivantes :
1. **Sauvegarde** : Les paramètres d'origine de l'ASLR, du niveau de restrictions du PMU (`perf_event_paranoid`, `kptr_restrict`), du Turbo Boost, ainsi que les gouverneurs, préférences d'énergie et états de veille des cœurs 1, 2 et 3 sont sauvegardés dans `/tmp` sur le serveur hôte.
2. **Désactivation du Turbo Boost** : Force la fréquence CPU à rester stable sans pics thermiques.
3. **Optimisation CPU (cœurs 1 à 3)** : Le gouverneur de ces trois cœurs passe en mode `performance` et les fréquences minimale et maximale sont verrouillées sur la fréquence maximale de base. L'algorithme matériel de gestion de l'énergie (EPP) est forcé sur `performance`. Le cœur 0 reste inchangé.
4. **Désactivation des Sleep States (C-States)** : Les états de veille profonds sont désactivés. Les cœurs restent actifs (C0), empêchant toute latence de réveil ou fluctuation de tension (DVFS).
5. **Déverrouillage des Compteurs Physiques (PMU)** : Définit temporairement `perf_event_paranoid` à `-1` et `kptr_restrict` à `0` sur l'hôte, permettant l'accès sans restriction aux compteurs de performance matérielle CPU (cycles, instructions, ratés de cache L1/L2/L3) depuis le conteneur.
6. **Désactivation de l'ASLR** : Désactive temporairement la randomisation de l'espace mémoire pour garantir un agencement identique d'une exécution à l'autre.
7. **Nettoyage des caches** : Force le vidage de la mémoire cache (page cache, dentries et inodes) pour démarrer les tests avec une mémoire propre.

Lors de la désactivation (`disable`), le script restaure automatiquement tous les paramètres d'origine sauvegardés lors du démarrage et relance les conteneurs applicatifs ainsi que les démons système.

## 6. Bonnes pratiques pour les benchmarks

Pendant que le Mode Laboratoire est actif :
1. **Rediriger la sortie standard** : Pour éviter que le chiffrement SSH en temps réel ne fausse les mesures CPU, écrivez les résultats dans un fichier :
   ```bash
   ./prysma_benchmark > results.txt
   ```
2. **Affinité CPU** : Exécutez le benchmark uniquement sur les cœurs optimisés (1 à 3) à l'aide de `taskset`, en laissant le cœur 0 au système d'exploitation :
   ```bash
   taskset -c 1-3 ./prysma_benchmark > results.txt
   ```

