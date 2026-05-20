# Automatisation via GitHub Actions (Runner)

L'exécution des tests de performance est automatisée avec un workflow GitHub Actions qui tourne sur un runner installé sur le serveur Debian 12.

Pour installer et enregistrer ce runner sur votre serveur, suivez le guide dédié : [configuration_runner.md](configuration_runner.md).

Le workflow exécute les opérations suivantes :
1. Construction de l'image de compilation éphémère.
2. Activation du Mode Laboratoire (isolation CPU, désactivation de l'ASLR, arrêt des services).
3. Lancement du conteneur avec `--privileged` et `--cpuset-cpus="1-3"` pour des mesures matérielles de haute fidélité.
4. Exécution locale du script `save_perf_results.py` pour enregistrer en base et notifier sur Discord.
5. Désactivation du Mode Laboratoire (restauration du serveur) systématique.

Exemple de job à intégrer dans le workflow `.github/workflows/ci.yml` :

```yaml
jobs:
  performance:
    runs-on: self-hosted
    steps:
      - name: Récupération du code
        uses: actions/checkout@v4

      - name: Copie des fichiers de construction sur l'hôte
        run: |
          cp docker/server/compiler/Dockerfile $HOME/prysma/Dockerfile
          cp docker/server/compiler/entrypoint.sh $HOME/prysma/entrypoint.sh

      - name: Construction de l'image éphémère
        run: docker build -t prysma-compiler docker/server/compiler

      - name: Activation du Mode Laboratoire
        run: |
          docker run --rm --privileged --pid=host \
            -v /var/run/docker.sock:/var/run/docker.sock \
            optiplex-lab-mode enable

      - name: Exécution des tests de performance (Haute Fidélité)
        run: |
          docker run --rm --cap-add=SYS_ADMIN --cpuset-cpus="1-3" \
            -v "$HOME/prysma":/prysma \
            -v "${{ github.workspace }}":/workspace \
            prysma-compiler "${{ github.event.pull_request.head.ref || github.ref_name }}"

      - name: Importation des résultats et notification Discord
        env:
          PRYSMA_DB_PATH: "/home/zyph/dashboard-prysma/prysma_perf.db"
          PRYSMA_CONFIG_PATH: "/home/zyph/dashboard-prysma/dashboard/dashboard_config.json"
        run: python3 tests/save_perf_results.py

      - name: Désactivation du Mode Laboratoire
        if: always()
        run: |
          docker run --rm --privileged --pid=host \
            -v /var/run/docker.sock:/var/run/docker.sock \
            optiplex-lab-mode disable

      - name: Correction des permissions du workspace
        if: always()
        run: docker run --rm -v "${{ github.workspace }}":/workspace alpine chown -R $(id -u):$(id -g) /workspace
```

> [!NOTE]
> * `PRYSMA_DB_PATH` : Emplacement de la base de données utilisée par le dashboard (ex : `/home/zyph/dashboard-prysma/prysma_perf.db`).
> * `PRYSMA_CONFIG_PATH` : Emplacement du fichier de configuration sécurisé contenant vos jetons Discord secrets (ex : `/home/zyph/dashboard-prysma/dashboard/dashboard_config.json`), évitant ainsi d'exposer vos secrets dans le dépôt Git public.
