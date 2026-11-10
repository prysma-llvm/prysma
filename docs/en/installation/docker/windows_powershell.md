# Installing Docker on Windows (PowerShell)

Setup guide for Docker Desktop on Windows using PowerShell with the WSL 2 backend for the Prysma compiler.

## 1. Enable WSL 2

Docker Desktop on Windows relies on the WSL 2 engine to deliver native Linux performance.

Open a **PowerShell terminal as Administrator**:

```powershell
wsl --install
```

If WSL is already installed, ensure it is updated and default to version 2:

```powershell
wsl --update
wsl --set-default-version 2
```

> If Windows prompts for a system restart to complete component installation, reboot before continuing.

## 2. Install Docker Desktop

You can install Docker Desktop directly through `winget`:

```powershell
winget install Docker.DockerDesktop
```

*(Alternatively, download the official installer from [docker.com](https://www.docker.com/products/docker-desktop/))*.

## 3. Configure Docker Desktop

1. Launch **Docker Desktop** from the Start menu.
2. Open Settings (gear icon in the top right corner).
3. Under **General**, verify that **"Use the WSL 2 based engine"** is enabled.
4. Under **Resources > WSL integration**, enable integration with your preferred Linux distribution if you also work inside WSL terminals.
5. Click **Apply & restart**.

## 4. Verification in PowerShell

Open a standard PowerShell window (administrator privileges not required) and verify the installation:

```powershell
docker --version
docker ps
```

## 5. PowerShell Specifics for Prysma

### Volume Mounting Syntax
In Windows PowerShell, use the `${PWD}` variable instead of Bash's `$(pwd)`:

```powershell
# Compile from the repository root
docker run --rm -v "${PWD}:/workspace" prysma-debug python3 build.py

# Interactive development session
docker run --rm -it -v "${PWD}:/workspace" prysma-debug
```

### Git Line Endings (CRLF vs LF)
Windows defaults to `CRLF` carriage returns. Because container bash scripts require `LF` endings, configure Git to preserve the correct format:

```powershell
git config --global core.autocrlf input
```

## See Also

* [Docker Compilation Guide](../docker_compilation.md): building images, compiling, and debugging.
* [Official Docker Desktop Documentation for Windows](https://docs.docker.com/desktop/setup/install/windows-install/)
