# configurer et lancer le docker de compilation sous arch linux

guide rapide pour installer docker sur arch, builder l'image de compilation de prysma, lancer les builds et rentrer dans le container en interactif si besoin de debug.

---

## 1. installer et activer docker sur arch linux

sur arch c'est tout con, on installe le paquet avec pacman :

```bash
sudo pacman -Syu docker
```

ensuite faut démarrer le démon docker et l'activer pour qu'il se lance au démarrage :

```bash
sudo systemctl enable --now docker.service
```

pour éviter de devoir taper `sudo` à chaque commande docker, ajoute ton utilisateur au groupe `docker` :

```bash
sudo usermod -aG docker $USER
```

applique le groupe directement sans te déco/reco :

```bash
newgrp docker
```

*(ou relance ta session si ça prend pas direct)*. test rapide pour vérifier que ça tourne :

```bash
docker ps
```

---

## 2. builder l'image du compiler prysma

**important :** le build doit se faire depuis la **racine du projet** `prysma` pour que le dockerfile puisse voir tous les fichiers du repo.

```bash
cd ~/prysma

# installation du docker de compilation
docker build -t prysma-compiler -f docker/compiler/Dockerfile .

# ou si tu l'as laissé dans docker/server/compiler/ :
# docker build -t prysma-compiler -f docker/server/compiler/Dockerfile .
```

ça va installer ubuntu 24.04, llvm-18, clang, python, etc. directement dans l'image.

---

## 3. commande pour lancer le docker

### lancer les tests de perf sur une branche (mode normal)

si tu veux lancer le runner sur une branche précise avec **tous les cœurs de ton CPU** :

```bash
cd ~/prysma

docker run --rm --cap-add=SYS_ADMIN \
  -v "$HOME/prysma":/prysma \
  -v "$(pwd)":/workspace \
  prysma-compiler main
```

*(remplace `main` par le nom de ta branche)*.

### lancer directement une commande sans cloner

si tu veux juste compiler le code local actuel monté dans le container :

```bash
docker run --rm \
  -v "$(pwd)":/workspace \
  --entrypoint python3 \
  prysma-compiler /workspace/build.py
```

---

## 4. commande pour entrer dans le docker (shell interactif)

comme l'image a un `ENTRYPOINT` configuré sur `entrypoint.sh`, pour rentrer directement dedans avec un bash interactif faut écraser l'entrypoint avec `--entrypoint /bin/bash` :

```bash
docker run --rm -it \
  --entrypoint /bin/bash \
  -v "$(pwd)":/workspace \
  prysma-compiler
```

une fois dedans, t'es dans `/workspace` avec llvm 18, clang et le venv déjà prêts dans le PATH. tu peux directement compiler prysma ou tester :

```bash
# compiler le projet
python3 build.py

# ou lancer les tests unitaires
python3 tests/run_unit_tests.py

# ou tester l'exécutable compilé
./build/Prysma --help
```

pour quitter le container :

```bash
exit
```

---

## 5. entrer dans un conteneur qui tourne déjà

si t'as déjà un conteneur en train de tourner en arrière-plan et que tu veux t'y connecter :

```bash
# trouver le container_id ou le nom
docker ps

# entrer dedans
docker exec -it <nom_ou_id_du_container> /bin/bash
```

---

## 6. compteurs matériels (PMU) sous arch

si le benchmark perf gueule parce qu'il a pas accès aux compteurs hardware :

```bash
sudo sysctl -w kernel.perf_event_paranoid=-1
sudo sysctl -w kernel.kptr_restrict=0
```
