# Laboratory Mode Configuration (OptiPlex Isolation)

This document describes how to deploy, build and run the control container to enable and disable Laboratory Mode on the OptiPlex server.

This mode allows shutting down background services (application Docker containers and the monitoring daemon) while keeping the Cloudflare tunnel and SSH service active to preserve remote connectivity.

## 1. File Structure

The container sources are located in the local repository:
* Executable file: `docker/server/lab_mode/lab_mode.sh`
* Build file: `docker/server/lab_mode/Dockerfile`

## 2. Deployment on the Server

Run the following commands to copy the configuration files to the execution folder on the server:

```bash
mkdir -p ~/lab-mode
cp docker/server/lab_mode/Dockerfile ~/lab-mode/Dockerfile
cp docker/server/lab_mode/lab_mode.sh ~/lab-mode/lab_mode.sh
```

## 3. Docker Image Build

Navigate to the target directory and build the Docker image:

```bash
cd ~/lab-mode
docker build -t optiplex-lab-mode .
```

## 4. Usage

To control the host's `systemd` services, the container uses the `nsenter` tool to execute commands directly in the host system context. The container must be run with PID namespace host access privileges (`--privileged --pid=host`).

### Enable Laboratory Mode (Quiet and Isolation)
This command stops the monitoring daemon and dynamically registers all running application containers to pause them, thus isolating the machine:

```bash
docker run --rm --privileged --pid=host \
  -v /var/run/docker.sock:/var/run/docker.sock \
  optiplex-lab-mode enable
```

### Disable Laboratory Mode (Server Restoration)
This command restarts the monitoring daemon and restores exactly the list of containers that were suspended during activation:

```bash
docker run --rm --privileged --pid=host \
  -v /var/run/docker.sock:/var/run/docker.sock \
  optiplex-lab-mode disable
```

## 5. Automatic System Stabilization

The Laboratory Mode activation script (`lab_mode.sh`) automatically configures the host to reduce jitter and measurement variance.

During activation (`enable`), the container applies the following rules:
1. **Backup**: The original ASLR parameters, PMU restriction levels (`perf_event_paranoid`, `kptr_restrict`), Turbo Boost, as well as the governors, energy preferences and sleep states of cores 1, 2 and 3 are saved in `/tmp` on the host server.
2. **Turbo Boost Disabling**: Forces the CPU frequency to remain stable without thermal spikes.
3. **CPU Optimization (cores 1 to 3)**: The governor of these three cores switches to `performance` mode and the minimum and maximum frequencies are locked to the base maximum frequency. The hardware energy management algorithm (EPP) is forced to `performance`. Core 0 remains unchanged.
4. **Sleep States (C-States) Disabling**: Deep sleep states are disabled. Cores remain active (C0), preventing any wake-up latency or voltage fluctuation (DVFS).
5. **Physical Counters (PMU) Unlocking**: Temporarily sets `perf_event_paranoid` to `-1` and `kptr_restrict` to `0` on the host, allowing unrestricted access to CPU hardware performance counters (cycles, instructions, L1/L2/L3 cache misses) from the container.
6. **ASLR Disabling**: Temporarily disables memory space randomization to guarantee an identical memory layout from one execution to another.
7. **Cache Cleaning**: Forces the flushing of cache memory (page cache, dentries and inodes) to start tests with clean memory.

During deactivation (`disable`), the script automatically restores all original parameters saved during startup and restarts application containers as well as system daemons.

## 6. Best Practices for Benchmarks

While Laboratory Mode is active:
1. **Redirect standard output**: To prevent real-time SSH encryption from skewing CPU measurements, write results to a file:
   ```bash
   ./prysma_benchmark > results.txt
   ```
2. **CPU Affinity**: Run the benchmark only on the optimized cores (1 to 3) using `taskset`, leaving core 0 to the operating system:
   ```bash
   taskset -c 1-3 ./prysma_benchmark > results.txt
   ```
