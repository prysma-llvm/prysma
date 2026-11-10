# Installing Docker on Debian

This guide covers Docker installation options on Debian (versions 11 Bullseye and 12 Bookworm).

## Preliminary Check

If Docker is already installed on your system, verify daemon status:

```bash
docker --version
docker ps
```

If the daemon responds without error, Docker is ready to use. You can skip directly to the [Docker Compilation Guide](../docker_compilation.md).

## Two Installation Approaches

* **Method 1: Standard Debian Package (`docker.io`) — Recommended for simplicity**  
  Maintained directly in Debian's official package repositories. Installs quickly without external repositories or third-party keys, and fully satisfies Prysma's compilation requirements.

* **Method 2: Upstream Official Repository (`docker-ce`)**  
  Provides the latest upstream releases built directly by Docker Inc. alongside official plugins.

## Method 1: Fast Installation via Debian Repositories (Recommended)

Install the standard distribution package:

```bash
sudo apt update
sudo apt install -y docker.io
```

Enable the systemd service and configure user permissions to avoid `sudo`:

```bash
sudo systemctl enable --now docker
sudo usermod -aG docker $USER
newgrp docker
```

Verify access:

```bash
docker ps
```

## Method 2: Installation via the Official Docker Repository (`docker-ce`)

### Package Conflict Prevention
If the native `docker.io` package was previously installed, remove it to prevent binary collisions in APT:

```bash
sudo apt remove -y docker.io containerd runc 2>/dev/null || true
```

### Adding the Official GPG Key and Repository
```bash
sudo apt update
sudo apt install -y ca-certificates curl
sudo install -m 0755 -d /etc/apt/keyrings

sudo curl -fsSL https://download.docker.com/linux/debian/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc

echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/debian \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

sudo apt update
```

### Installation and Activation
```bash
sudo apt install -y docker-ce docker-ce-cli containerd.io
sudo systemctl enable --now docker
sudo usermod -aG docker $USER
newgrp docker
```

Verify service connectivity:

```bash
docker ps
```

## See Also

* [Docker Compilation Guide](../docker_compilation.md): image creation, compilation, and debugging.
* [Official Docker Documentation for Debian](https://docs.docker.com/engine/install/debian/)
