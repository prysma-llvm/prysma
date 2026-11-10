# Installing Docker on Arch Linux

Setup guide for the Docker engine on Arch Linux for the Prysma compilation environment.

## 1. Package Installation

On Arch Linux, install the `docker` package:

```bash
sudo pacman -Syu docker
```

## 2. System Service Activation

Enable the daemon to launch on boot and start it immediately:

```bash
sudo systemctl enable --now docker.service
```

## 3. User Permission Configuration (Non-root Execution)

Add your user to the `docker` group to execute commands without `sudo`:

```bash
sudo usermod -aG docker $USER
newgrp docker
```

> If changes are not recognized immediately in all terminals, restart your user session.

## 4. Verifying the Installation

Confirm that the Docker daemon is active and accessible:

```bash
docker ps
```

If the command lists running containers without permission errors, Docker is ready.

## See Also

* [Docker Compilation Guide](../docker_compilation.md): building images, compiling, and debugging.
* [Official ArchWiki Docker Documentation](https://wiki.archlinux.org/title/Docker)
