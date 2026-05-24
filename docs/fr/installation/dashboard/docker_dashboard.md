# Déploiement du Dashboard de Performance

Le dashboard utilise Streamlit pour présenter les graphiques d'évolution des performances et l'historique des exécutions.

## 1. Structure des fichiers

Sur le serveur, le dossier de déploiement `~/dashboard-prysma/` contient les éléments à plat :

```
~/dashboard-prysma/
├── Dockerfile
├── docker-compose.yml
├── dashboard/
│   ├── app.py
│   ├── style.css
│   └── dashboard_config.json
├── perf_framework/
├── prysma_perf.db
└── save_perf_results.py
```

* `dashboard/app.py` : Point d'entrée Streamlit, contient toute la logique de l'interface.
* `dashboard/style.css` : Feuille de style personnalisée pour le thème sombre.
* `dashboard/dashboard_config.json` : Configuration des seuils de régression et des notifications Discord.
* `Dockerfile` : Image de base Python 3.12 avec Streamlit, pandas, plotly et sqlite3.
* `docker-compose.yml` : Orchestration du conteneur. Monte le répertoire courant dans `/workspace` pour assurer la synchronisation en temps réel de la base SQLite et des fichiers.
* `perf_framework/` : Module Python contenant la logique de compilation, de profilage, de base de données et de notifications.
* `prysma_perf.db` : Base de données SQLite générée automatiquement.

Les fichiers sources (`Dockerfile`, `docker-compose.yml`) se trouvent dans `docker/server/dashboard/` dans le dépôt et doivent être copiés dans `~/dashboard-prysma/`.

## 2. Configuration des seuils et notifications

Le fichier de configuration `dashboard/dashboard_config.json` définit les seuils de régression ainsi que l'accès au salon Discord :

```json
{
  "thresholds": {
    "instructions": 0.5,
    "cycles": 5.0,
    "CPI": 4.0,
    "l1_dcache_misses": 10.0,
    "l2_cache_misses": 10.0,
    "l3_cache_misses": 10.0,
    "ram_accesses": 10.0,
    "branch_misses": 5.0,
    "peak_rss": 5.0
  },
  "notifications": {
    "enabled": true,
    "discord_bot": {
      "bot_token": "TON_TOKEN_ICI",
      "channel_id": "TON_CHANNEL_ICI"
    }
  }
}
```

## 3. Lancement du service

Depuis le dossier de déploiement du dashboard sur le serveur :

```bash
docker-compose up -d --build
```

L'application est ensuite accessible sur le port **8501** (ex : `http://localhost:8501`).

## 4. Gestion du conteneur

* **Arrêter le service** :
  ```bash
  docker-compose down
  ```
* **Consulter les logs** :
  ```bash
  docker-compose logs -f
  ```
* **Redémarrer le service** :
  ```bash
  docker-compose restart
  ```

## 5. Exemple d'importation des résultats de tests

Pour importer manuellement des données de performance générées par le compilateur dans la base de données du dashboard, exécutez la commande suivante depuis le dossier `~/dashboard-prysma` :

```bash
cd ~/dashboard-prysma
python3 save_perf_results.py ../prysma/prysma/perf_run_data.json
```

**Exemple de sortie réussie :**
```text
Importing results for commit: unknown
Database updated: $HOME/dashboard-prysma/prysma_perf.db
Successfully removed temporary results file: ../prysma/prysma/perf_run_data.json
```
