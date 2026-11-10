# Installation de Docker sur Ubuntu

Ce guide présente les options d'installation de Docker sous Ubuntu (20.04, 22.04 et 24.04 LTS) adaptées à votre environnement de travail.

## Vérification préalable

Si Docker est déjà présent sur votre machine, testez simplement son fonctionnement :

```bash
docker --version
docker ps
```

Si ces commandes s'exécutent sans erreur, Docker est déjà opérationnel. Vous n'avez rien à modifier ou réinstaller et pouvez directement consulter le [Guide de compilation Docker](../docker_compilation.md).

## Deux approches possibles

Il existe deux manières d'installer Docker sous Ubuntu. Pour compiler et tester Prysma, les deux fonctionnent à l'identique :

* **Méthode 1 : Paquet standard Ubuntu (`docker.io`) — Recommandée pour sa simplicité**  
  Ce paquet est directement maintenu par les équipes d'Ubuntu dans les dépôts officiels. Il s'installe en une seule commande, sans ajouter de dépôt tiers ni de clé GPG externe. Il prend parfaitement en charge les multi-stage builds et le montage de volumes requis par Prysma.

* **Méthode 2 : Dépôt officiel upstream (`docker-ce`)**  
  Utile si vous souhaitez la toute dernière version publiée directement par l'entreprise Docker ou des plugins annexes récents (`buildx`, `compose`).

## Méthode 1 : Installation via les dépôts Ubuntu (recommandé)

Installez le paquet natif :

```bash
sudo apt update
sudo apt install -y docker.io
```

Activez le service et configurez les droits d'accès pour exécuter Docker sans `sudo` :

```bash
sudo systemctl enable --now docker
sudo usermod -aG docker $USER
newgrp docker
```

Vérifiez que le démon est accessible :

```bash
docker ps
```

## Méthode 2 : Installation via le dépôt officiel Docker (`docker-ce`)

Cette méthode configure le dépôt maintenu par Docker Inc.

### Prévention des conflits de paquets
Les paquets `docker.io` (fourni par Ubuntu) et `docker-ce` (fourni par Docker) installent le même exécutable `/usr/bin/docker`. Si `docker.io` était déjà présent, désinstallez-le pour éviter un conflit dans APT :

```bash
sudo apt remove -y docker.io containerd runc 2>/dev/null || true
```

### Ajout de la clé GPG et du dépôt officiel
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

### Installation et activation
```bash
sudo apt install -y docker-ce docker-ce-cli containerd.io
sudo systemctl enable --now docker
sudo usermod -aG docker $USER
newgrp docker
```

Vérifiez l'accès :

```bash
docker ps
```

## Voir aussi

* [Guide de compilation Docker](../docker_compilation.md) : construction des images, compilation et débogage.
* [Documentation officielle Docker pour Ubuntu](https://docs.docker.com/engine/install/ubuntu/)
