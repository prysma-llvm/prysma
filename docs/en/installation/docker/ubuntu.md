# Installing Docker on Ubuntu

This guide outlines Docker installation options on Ubuntu (20.04, 22.04, and 24.04 LTS).

## Preliminary Check

If Docker is already present on your machine, test its status:

```bash
docker --version
docker ps
```

If these commands execute without errors, Docker is already operational. You do not need to install or reconfigure anything; you can proceed directly to the [Docker Compilation Guide](../docker_compilation.md).

## Two Installation Approaches

Two methods are available on Ubuntu. For compiling and developing Prysma, both operate identically:

* **Method 1: Standard Ubuntu Package (`docker.io`) — Recommended for simplicity**  
  Directly maintained by Canonical in Ubuntu's official repositories. Installs via a single command without third-party repositories or external GPG keys. It fully supports multi-stage builds and volume mounts required by Prysma.

* **Method 2: Upstream Official Repository (`docker-ce`)**  
  Useful if you require the latest upstream releases from Docker Inc. or recent ancillary plugins (`buildx`, `compose`).

## Method 1: Fast Installation via Ubuntu Repositories (Recommended)

Install the native package:

```bash
sudo apt update
sudo apt install -y docker.io
```

Enable the service and add your user to the `docker` group to run commands without `sudo`:

```bash
sudo systemctl enable --now docker
sudo usermod -aG docker $USER
newgrp docker
```

Verify that the daemon is reachable:

```bash
docker ps
```

## Method 2: Installation via the Official Docker Repository (`docker-ce`)

This method configures the repository maintained by Docker Inc.

### Package Conflict Prevention
Both `docker.io` (provided by Ubuntu) and `docker-ce` (provided by Docker) provide the `/usr/bin/docker` binary. If `docker.io` was previously installed, remove it to avoid package manager collisions:

```bash
sudo apt remove -y docker.io containerd runc 2>/dev/null || true
```

### Adding the Official GPG Key and Repository
```bash
sudo apt update
sudo apt install -y ca-certificates curl
sudo install -m 0755 -d /etc/apt/keyrings

sudo curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc

echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

sudo apt update
```

### Installation and Service Setup
```bash
sudo apt install -y docker-ce docker-ce-cli containerd.io
sudo systemctl enable --now docker
sudo usermod -aG docker $USER
newgrp docker
```

Verify the installation:

```bash
docker ps
```

## See Also

* [Docker Compilation Guide](../docker_compilation.md): image creation, compilation, and debugging.
* [Official Docker Documentation for Ubuntu](https://docs.docker.com/engine/install/ubuntu/)
