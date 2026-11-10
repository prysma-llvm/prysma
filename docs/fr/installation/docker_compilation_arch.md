# Installation de Docker sur Arch Linux

Guide de configuration de Docker sous Arch Linux pour le projet Prysma.

## 1. Installation du paquet

Installez le paquet `docker` avec `pacman` :

```bash
sudo pacman -Syu docker
```

## 2. Activation du service système

Activez et démarrez le démon Docker :

```bash
sudo systemctl enable --now docker.service
```

## 3. Configuration des privilèges (sans sudo)

Ajoutez votre utilisateur au groupe `docker` :

```bash
sudo usermod -aG docker $USER
newgrp docker
```

## 4. Vérification du démon

```bash
docker ps
```

## Voir aussi

* [Guide de compilation Docker](docker_compilation.md) : commandes de build, debug et exécution interactive.
* [Documentation officielle ArchWiki (Docker)](https://wiki.archlinux.org/title/Docker)
