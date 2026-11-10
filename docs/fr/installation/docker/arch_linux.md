# Installation de Docker sur Arch Linux

Guide de configuration du moteur Docker sur Arch Linux pour l'environnement de compilation Prysma.

## 1. Installation du paquet

Sur Arch Linux, installez le paquet `docker` :

```bash
sudo pacman -Syu docker
```

## 2. Activation du service système

Activez le démon pour qu'il démarre automatiquement au boot et lancez-le immédiatement :

```bash
sudo systemctl enable --now docker.service
```

## 3. Configuration des privilèges utilisateur (sans sudo)

Ajoutez votre utilisateur au groupe `docker` afin d'exécuter les commandes sans élévation de privilèges :

```bash
sudo usermod -aG docker $USER
```

Appliquez immédiatement le changement au shell courant sans redémarrer la session :

```bash
newgrp docker
```

> Si la modification n'est pas reconnue immédiatement par certains terminaux, redémarrez votre session utilisateur.

## 4. Vérification de l'installation

Vérifiez que le démon Docker est actif et accessible :

```bash
docker ps
```

Une fois la commande exécutée sans erreur de permissions, Docker est opérationnel.

## Voir aussi

* [Guide de compilation Docker](../docker_compilation.md) : construction des images, compilation et débogage.
* [Documentation officielle ArchWiki (Docker)](https://wiki.archlinux.org/title/Docker)
