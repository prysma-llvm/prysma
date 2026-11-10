# Installation de Docker sur Debian

Ce guide présente les options d'installation de Docker sous Debian (versions 11 Bullseye et 12 Bookworm).

## Vérification préalable

Si Docker est déjà installé sur votre système, testez son accessibilité :

```bash
docker --version
docker ps
```

Si le démon répond sans erreur, Docker est prêt à l'emploi. Vous pouvez passer directement au [Guide de compilation Docker](../docker_compilation.md).

## Deux approches possibles

* **Méthode 1 : Paquet standard Debian (`docker.io`) — Recommandée pour sa simplicité**  
  Maintenu directement dans les dépôts officiels de Debian. S'installe rapidement sans ajouter de clés ou de dépôts tiers, et suffit amplement pour compiler Prysma.

* **Méthode 2 : Dépôt officiel upstream (`docker-ce`)**  
  Fournit les versions les plus récentes compilées par Docker Inc. avec les plugins officiels.

## Méthode 1 : Installation via les dépôts Debian (recommandé)

Installez le paquet standard :

```bash
sudo apt update
sudo apt install -y docker.io
```

Activez le service systemd et configurez les droits utilisateur pour éviter `sudo` :

```bash
sudo systemctl enable --now docker
sudo usermod -aG docker $USER
newgrp docker
```

Vérifiez l'accès :

```bash
docker ps
```

## Méthode 2 : Installation via le dépôt officiel Docker (`docker-ce`)

### Prévention des conflits de paquets
Si le paquet natif `docker.io` était déjà installé, retirez-le pour éviter un conflit de binaires dans APT :

```bash
sudo apt remove -y docker.io containerd runc 2>/dev/null || true
```

### Configuration de la clé GPG et du dépôt officiel
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

### Installation et activation
```bash
sudo apt install -y docker-ce docker-ce-cli containerd.io
sudo systemctl enable --now docker
sudo usermod -aG docker $USER
newgrp docker
```

Vérifiez que le service répond :

```bash
docker ps
```

## Voir aussi

* [Guide de compilation Docker](../docker_compilation.md) : construction des images, compilation et débogage.
* [Documentation officielle Docker pour Debian](https://docs.docker.com/engine/install/debian/)
