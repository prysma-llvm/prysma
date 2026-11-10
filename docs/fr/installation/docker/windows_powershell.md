# Installation de Docker sous Windows (PowerShell)

Guide de configuration de Docker Desktop sous Windows via PowerShell avec le backend WSL 2 pour le compilateur Prysma.

## 1. Activation de WSL 2

Docker Desktop sous Windows s'appuie sur le moteur WSL 2 pour offrir les performances d'un environnement Linux natif.

Dans un terminal **PowerShell exécuté en mode Administrateur** :

```powershell
wsl --install
```

Si WSL est déjà installé, assurez-vous qu'il est à jour et réglé sur la version 2 :

```powershell
wsl --update
wsl --set-default-version 2
```

> Si Windows demande un redémarrage système pour finaliser l'installation des composants, redémarrez avant de poursuivre.

## 2. Installation de Docker Desktop

Installez Docker Desktop via le gestionnaire de paquets `winget` :

```powershell
winget install Docker.DockerDesktop
```

*(Alternativement, téléchargez l'exécutable officiel depuis [docker.com](https://www.docker.com/products/docker-desktop/))*.

## 3. Configuration de Docker Desktop

1. Lancez **Docker Desktop** depuis le menu Démarrer.
2. Ouvrez les paramètres (icône d'engrenage en haut à droite).
3. Dans **General**, vérifiez que l'option **"Use the WSL 2 based engine"** est activée.
4. Dans **Resources > WSL integration**, cochez votre distribution Linux par défaut si vous travaillez aussi sous un terminal WSL.
5. Cliquez sur **Apply & restart**.

## 4. Vérification dans PowerShell

Ouvrez un terminal PowerShell standard (droits administrateur non requis) et vérifiez la commande :

```powershell
docker --version
docker ps
```

## 5. Particularités PowerShell pour Prysma

### Syntaxe de montage des volumes
Sous Windows PowerShell, la variable `${PWD}` doit être utilisée à la place de la syntaxe Bash `$(pwd)` :

```powershell
# Compilation depuis la racine du dépôt
docker run --rm -v "${PWD}:/workspace" prysma-debug python3 build.py

# Session interactive
docker run --rm -it -v "${PWD}:/workspace" prysma-debug
```

### Fins de ligne Git (CRLF vs LF)
Windows utilise par défaut des retours chariot `CRLF`. Les scripts du conteneur Linux attendant des fins de ligne `LF`, configurez Git pour préserver le format adapté :

```powershell
git config --global core.autocrlf input
```

## Voir aussi

* [Guide de compilation Docker](../docker_compilation.md) : construction des images, compilation et débogage.
* [Documentation officielle Docker Desktop pour Windows](https://docs.docker.com/desktop/setup/install/windows-install/)
