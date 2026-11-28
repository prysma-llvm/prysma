# Premier programme en Prysma

Ce guide part du dépôt Prysma et montre comment compiler puis lancer un fichier `.p`.

## Préparer le compilateur

Il te faut [Docker](docker_compilation.md) et Git. Dans un terminal, clone le dépôt et construis l’image de développement :

```sh
git clone https://github.com/prysma-llvm/prysma.git
cd prysma
docker build --target debug -t prysma-debug -f docker/compiler/Dockerfile .
```

Depuis la racine du dépôt, ouvre un terminal dans le conteneur. La commande monte le dépôt local dans `/workspace`.

**Bash (Linux ou WSL)**

```sh
docker run --rm -it --user "$(id -u):$(id -g)" -e HOME=/tmp -v "$PWD:/workspace" prysma-debug bash
```

**Windows PowerShell** avec Docker Desktop en mode conteneurs Linux

```powershell
docker run --rm -it -v "$($PWD.Path):/workspace" prysma-debug bash
```

Dans le conteneur, lance `python3 debug.py` pour construire `build/Prysma`. Tu peux sauter cette étape si le compilateur est déjà construit.

## Écrire et lancer le programme

Dans ton éditeur, crée `hello.p` à la racine du dépôt :

```prysma
fn int32 main() {
    dec int32 result = 40 + 2;
    call print(result);
    call backSlashN();
    return 0;
}
```

`main` est le point d’entrée du programme. `print` affiche le résultat, `backSlashN()` ajoute un retour à la ligne et `return 0` indique que le programme s’est terminé normalement.

Retourne dans le terminal du conteneur et lance :

```sh
./build/Prysma hello.p && ./build/hello
```

La première commande compile le fichier. La seconde exécute le binaire `build/hello`. Tu devrais voir :

```text
42
```

## Vidéo : Fibonacci en Prysma

Ce [tutoriel vidéo](https://youtu.be/0pbe04c52Qw) montre comment écrire, compiler et exécuter un programme qui affiche les dix premiers termes de la suite de Fibonacci en Prysma. (1 min 5 s, texte à l’écran en anglais)

Pour aller plus loin : [spécification du langage](../language_specification.md).
