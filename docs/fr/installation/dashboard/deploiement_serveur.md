# Déploiement du serveur Prysma (Debian 12)

Ce document décrit l'installation et la configuration de l'infrastructure de performance sur le serveur Debian 12. Les conteneurs s'appuient sur une base Ubuntu 24.04 afin de faciliter l'intégration de LLVM 18.

## 1. Sécurité et séparation des privilèges

Pour sécuriser l'exécution des tests de performance (notamment face à du code non approuvé lors des Pull Requests), le système sépare strictement les accès :

1. **Le conteneur de test (Docker éphémère)** :
   - Exécute uniquement le script `tests/run_perf_tests.py`.
   - Ne contient aucun token Discord ni accès en écriture à la base de données.
   - Compile le projet, lance les benchmarks et écrit les mesures dans le fichier `perf_run_data.json` à la racine.
   - Le conteneur se détruit automatiquement après l'exécution (`--rm`).

2. **Le script d'importation (sur l'hôte sécurisé)** :
   - Exécute le script `save_perf_results.py`.
   - Lit le fichier `perf_run_data.json` généré.
   - Enregistre les métriques dans la base SQLite `prysma_perf.db` et gère les notifications Discord via le token configuré.

## 2. Configuration de Docker sur Debian 12

Si Docker n'est pas installé sur le serveur hôte :

```bash
sudo apt update
sudo apt install -y docker.io docker-compose-v2
sudo usermod -aG docker $USER
newgrp docker
```
