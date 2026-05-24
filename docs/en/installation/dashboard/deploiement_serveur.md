# Prysma Server Deployment (Debian 12)

This document describes the installation and configuration of the performance infrastructure on the Debian 12 server. The containers are based on Ubuntu 24.04 to facilitate the integration of LLVM 18.

## 1. Security and Privilege Separation

To secure the execution of performance tests (especially against unapproved code during Pull Requests), the system strictly separates access:

1. **The test container (ephemeral Docker)**:
   - Executes only the `tests/run_perf_tests.py` script.
   - Contains no Discord token nor write access to the database.
   - Compiles the project, runs the benchmarks and writes the measurements to the `perf_run_data.json` file at the root.
   - The container is automatically destroyed after execution (`--rm`).

2. **The import script (on the secured host)**:
   - Executes the `save_perf_results.py` script.
   - Reads the generated `perf_run_data.json` file.
   - Records the metrics in the SQLite database `prysma_perf.db` and manages Discord notifications via the configured token.

## 2. Docker Configuration on Debian 12

If Docker is not installed on the host server:

```bash
sudo apt update
sudo apt install -y docker.io docker-compose-v2
sudo usermod -aG docker $USER
newgrp docker
```
