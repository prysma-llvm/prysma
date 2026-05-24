# Automation via GitHub Actions (Runner)

The execution of performance tests is automated with a GitHub Actions workflow running on a runner installed on the Debian 12 server.

To install and register this runner on your server, follow the dedicated guide: [configuration_runner.md](configuration_runner.md).

The workflow executes the following operations:
1. Building the ephemeral compilation image.
2. Enabling Laboratory Mode (CPU isolation, ASLR disabling, service shutdown).
3. Launching the container with `--privileged` and `--cpuset-cpus="1-3"` for high-fidelity hardware measurements.
4. Local execution of the `save_perf_results.py` script to record to the database and notify on Discord.
5. Systematic disabling of Laboratory Mode (server restoration).

Example job to integrate into the `.github/workflows/ci.yml` workflow:

```yaml
jobs:
  performance:
    runs-on: self-hosted
    steps:
      - name: Code Checkout
        uses: actions/checkout@v4

      - name: Copy Build Files to Host
        run: |
          cp docker/server/compiler/Dockerfile $HOME/prysma/Dockerfile
          cp docker/server/compiler/entrypoint.sh $HOME/prysma/entrypoint.sh

      - name: Build Ephemeral Image
        run: docker build -t prysma-compiler docker/server/compiler

      - name: Enable Laboratory Mode
        run: |
          docker run --rm --privileged --pid=host \
            -v /var/run/docker.sock:/var/run/docker.sock \
            optiplex-lab-mode enable

      - name: Run Performance Tests (High Fidelity)
        run: |
          docker run --rm --cap-add=SYS_ADMIN --cpuset-cpus="1-3" \
            -v "$HOME/prysma":/prysma \
            -v "${{ github.workspace }}":/workspace \
            prysma-compiler "${{ github.event.pull_request.head.ref || github.ref_name }}"

      - name: Import Results and Discord Notification
        env:
          PRYSMA_DB_PATH: "/home/zyph/dashboard-prysma/prysma_perf.db"
          PRYSMA_CONFIG_PATH: "/home/zyph/dashboard-prysma/dashboard/dashboard_config.json"
        run: python3 tests/save_perf_results.py

      - name: Disable Laboratory Mode
        if: always()
        run: |
          docker run --rm --privileged --pid=host \
            -v /var/run/docker.sock:/var/run/docker.sock \
            optiplex-lab-mode disable

      - name: Fix Workspace Permissions
        if: always()
        run: docker run --rm -v "${{ github.workspace }}":/workspace alpine chown -R $(id -u):$(id -g) /workspace
```

> [!NOTE]
> * `PRYSMA_DB_PATH`: Location of the database used by the dashboard (e.g.: `/home/zyph/dashboard-prysma/prysma_perf.db`).
> * `PRYSMA_CONFIG_PATH`: Location of the secure configuration file containing your secret Discord tokens (e.g.: `/home/zyph/dashboard-prysma/dashboard/dashboard_config.json`), thus avoiding exposing your secrets in the public Git repository.
