# Automatisation via GitHub Actions (Runner)

L'exécution des tests de performance est automatisée avec un workflow GitHub Actions qui tourne sur un runner installé sur le serveur Debian 12.

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

      - name: Construction de l'image éphémère
        run: docker build -t prysma-compiler -f docker/server/compiler/Dockerfile .

      - name: Activation du Mode Laboratoire
        run: |
          docker run --rm --privileged --pid=host \
            -v /var/run/docker.sock:/var/run/docker.sock \
            optiplex-lab-mode enable

      - name: Exécution des tests de performance (Haute Fidélité)
        run: docker run --rm --privileged --cpuset-cpus="1-3" -v "${{ github.workspace }}":/workspace prysma-compiler

      - name: Importation des résultats et notification Discord
        env:
          PRYSMA_DB_PATH: "/home/zyph/dashboard-prysma/prysma_perf.db"
        run: python3 tests/save_perf_results.py

      - name: Désactivation du Mode Laboratoire
        if: always()
        run: |
          docker run --rm --privileged --pid=host \
            -v /var/run/docker.sock:/var/run/docker.sock \
            optiplex-lab-mode disable

      - name: Fix workspace permissions
        if: always()
        run: sudo chown -R $(id -u):$(id -g) "${{ github.workspace }}"
```

> [!NOTE]
> Le chemin `PRYSMA_DB_PATH` doit correspondre exactement à l'emplacement de la base de données utilisée par votre conteneur dashboard (ici `/home/zyph/dashboard-prysma/prysma_perf.db`) pour que le dashboard affiche immédiatement les nouveaux résultats.
