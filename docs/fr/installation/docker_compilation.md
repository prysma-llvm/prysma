# Architecture de centralisation et compilation Docker

Ce document décrit l'architecture de compilation de Prysma ainsi que les commandes pour construire les images Docker, compiler le projet et travailler en mode interactif.

## 1. Architecture de centralisation

Prysma est un compilateur système qui s'appuie sur une chaîne d'outils (toolchain) complète :

* **LLVM 18** et ses bibliothèques de développement (`llvm-18`, `llvm-18-dev`, `libclang-rt-18-dev`)
* Compilateur **Clang 18** et éditeur de liens **LLD 18**
* **C++23**, **CMake** et **ccache** pour l'accélération des compilations incrémentales
* Outils d'analyse et d'introspection : **Bear** (génération de `compile_commands.json`) et **Graphviz**
* Débogage natif : **GDB**, **LLDB-18** et **debugpy** pour l'outillage Python
* Environnement virtuel Python 3 dédié avec les dépendances du compilateur

L'installation locale manuelle de ces composants engendre régulièrement des dérives de versions, des problèmes de configuration d'ABI et des difficultés de portabilité selon les distributions.

L'environnement officiel utilise **Docker comme architecture de centralisation** :
- **Chaîne d’outils verrouillée** : le Dockerfile utilise une image Ubuntu référencée par digest, une archive APT datée et des paquets Python vérifiés par SHA-256. Reconstruire la même révision ne met pas les dépendances à jour implicitement.
- **Aucune dépendance lourde sur l'hôte** : aucun paquet LLVM ou bibliothèque tierce n'est requis sur le système hôte. Docker suffit pour compiler dans le conteneur ; les tests sur l’hôte et VS Code demandent des outils supplémentaires.
- **Prêt à l'emploi** : le conteneur intègre les variables d'environnement, les chemins LLVM et le venv Python préconfigurés.

## 2. Prérequis : Installer Docker sur l'hôte

Consultez le guide correspondant à votre système pour installer et activer le moteur Docker :

* [Installation sur Arch Linux](docker/arch_linux.md)
* [Installation sur Debian](docker/debian.md)
* [Installation sur Ubuntu](docker/ubuntu.md)
* [Installation sur Windows (PowerShell)](docker/windows_powershell.md)

## 3. Construction des images Docker

> **Important :** Lancez le build depuis la racine de `prysma`. Les deux cibles sont verrouillées sur `linux/amd64`, y compris sous Windows/WSL2. Une machine ARM doit disposer d’une émulation x86-64.

Le Dockerfile est structuré en multi-stage build afin de produire des cibles adaptées à chaque usage :

```bash
# Image de développement & débogage (gdb, lldb-18, bear, graphviz, python venv)
docker build --target debug -t prysma-debug -f docker/compiler/Dockerfile .

# Image pour le serveur de benchmarks (linux perf tools)
docker build --target benchmark -t prysma-benchmark -f docker/compiler/Dockerfile .
```

Les deux cibles partagent la même couche de base (Ubuntu 24.04, LLVM 18, Clang, Python), ce qui optimise l'utilisation du cache Docker lors de la reconstruction.

Pour mettre la chaîne à jour, révisez le digest Ubuntu et la date du snapshot dans `docker/compiler/ubuntu-snapshot.sources`, ainsi que l’URL et le SHA-256 du paquet de certificats utilisé au démarrage si nécessaire. Synchronisez `docker/compiler/requirements.lock` avec le `requirements.txt` de l’hôte et vérifiez les hashes des roues pour CPython 3.12 et Linux x86-64. Reconstruisez puis testez les images avant de déployer la mise à jour.

Ce verrouillage fixe les entrées de build, pas l’empreinte binaire de l’image reconstruite : les métadonnées de build peuvent différer. Pour distribuer exactement la même image, publiez l’image validée et utilisez son digest de registre.

## 4. Orchestration de la compilation (`build.py` et `debug.py`)

La compilation à l'intérieur du conteneur est orchestrée par deux wrappers Python : `build.py` et `debug.py`.

### Rôle du wrapper Python
* **Génération de code automatisée en amont** : Avant d'invoquer la compilation C++, le script exécute les générateurs du compilateur (`GeneratorAST`, `GeneratorInterfaceVisitor`, `GeneratorVisitorBaseGeneral`, `GeneratorGraphViz`, `GeneratorExpression`, `GeneratorParser`) pour produire automatiquement les classes AST, les visiteurs et la grammaire.
* **Configuration dynamique** : Il configure `ccache` (taille max de 10 Go), détecte le nombre de cœurs CPU disponibles (`--parallel`), et injecte les drapeaux de compilation adaptés.

### Modes de compilation
* **`build.py` (Mode Release)** : Compile avec les optimisations `-O3`, `-march=native`, `-ffast-math`, `-fomit-frame-pointer`, `-flto`, et l'éditeur de liens `-fuse-ld=lld` avec élimination des sections mortes (`--gc-sections`).
* **`debug.py` (Mode Debug)** : Compile avec `-g3`, `-O0`, `-fno-omit-frame-pointer`, `-fstack-protector-all`, les avertissements stricts, ainsi que les détecteurs de corruption mémoire **AddressSanitizer** (`-fsanitize=address`) et **UndefinedBehaviorSanitizer** (`-fsanitize=undefined`).

## 5. Commandes de compilation directes depuis l'hôte

Le code source local est monté dans le répertoire `/workspace` du conteneur. Les commandes peuvent être invoquées directement depuis le terminal hôte :

```bash
# Compiler Prysma (build de release optimisé avec ccache)
docker run --rm -v "$(pwd)":/workspace prysma-debug python3 build.py

# Compiler en mode debug (avec sanitizers et symboles de débogage injectés)
docker run --rm -v "$(pwd)":/workspace prysma-debug python3 debug.py

# Exécuter les tests unitaires
docker run --rm -v "$(pwd)":/workspace prysma-debug python3 tests/run_unit_tests.py --existing

# Exécuter le binaire compilé
docker run --rm -v "$(pwd)":/workspace prysma-debug ./build/Prysma tests/prysma_code_tests/project_test_edge_case/test_recursion/test_simple_recursion.p
```

> **Windows PowerShell :** Remplacez `"$(pwd)"` par `"${PWD}"`.

## 6. Session interactive (Shell dans le conteneur)

Pour travailler directement dans l'environnement centralisé :

```bash
docker run --rm -it -v "$(pwd)":/workspace prysma-debug
```

À l'intérieur du conteneur, l'invite se place dans `/workspace` avec LLVM 18, Clang et l'environnement virtuel Python chargés dans le `PATH` :

```bash
# Compilation en mode release
python3 build.py

# Ou compilation en mode debug (sanitizers injectés)
python3 debug.py

# Exécution des tests unitaires
python3 tests/run_unit_tests.py --existing

# Test du compilateur compilé
./build/Prysma tests/prysma_code_tests/project_test_edge_case/test_recursion/test_simple_recursion.p

# Quitter le conteneur
exit
```

## 7. Connexion à un conteneur en cours d'exécution

Si un conteneur tourne déjà en arrière-plan et qu'une session de débogage est nécessaire :

```bash
# 1. Identifier le conteneur cible
docker ps

# 2. Ouvrir un shell interactif dans ce conteneur
docker exec -it <nom_ou_id_du_conteneur> /bin/bash
```

## 8. Exécution des benchmarks de performance

Pour exécuter la suite de tests de performance sur une branche Git avec l'ensemble des cœurs CPU :

```bash
docker run --rm --cap-add=SYS_ADMIN \
  -v "/tmp/prysma-bench":/prysma \
  -v "$(pwd)":/workspace \
  prysma-benchmark <nom_de_la_branche>
```

## 9. Accès aux compteurs matériels (PMU / Linux perf)

Si l'outil de benchmark signale un blocage d'accès aux compteurs matériels du processeur hôte (cycles, ratés de cache L1/L2/L3), ajustez temporairement les restrictions du noyau Linux sur l'hôte :

```bash
sudo sysctl -w kernel.perf_event_paranoid=-1
sudo sysctl -w kernel.kptr_restrict=0
```

## 10. Intégration VS Code et wrappers Docker CLI

Le dépôt inclut des scripts wrappers par plateforme (`docker/environnement_dev/linux/docker-cli.sh` et `docker/environnement_dev/windows/docker-cli.ps1`) ainsi qu'une configuration prête à l'emploi dans `.vscode/tasks.json` :

* **Détection d'environnement** : Détecte automatiquement l'environnement d'exécution (VS Code natif ou sandbox Flatpak via `flatpak-spawn`).
* **Gestion des permissions** : Exécute les commandes avec votre UID/GID hôte (`--user $(id -u):$(id -g)`), garantissant que les fichiers générés dans `build/` restent modifiables sans `sudo`.
* **Indexation Clangd** : Réécrit automatiquement les chemins `/workspace` vers le chemin absolu de la machine hôte dans `build/compile_commands.json` et relie les en-têtes LLVM (`.clangd-includes`), assurant l'autocomplétion et le diagnostic dans VS Code sans nécessiter LLVM sur l'hôte.

Pour la configuration du profil `.vscode/Prysma.code-profile`, l'installation de l'extension de coloration syntaxique et le débogage interactif, voir le [Guide de l'environnement VS Code](vscode_environment.md).

La sélection `--existing` exécute les suites du compilateur dans Docker. Les tests de l’environnement doivent être lancés sur l’hôte avec `python3 tests/run_unit_tests.py --dev-env` ; consultez les [prérequis et limites](../../../tests/environnement_dev_tests/README.md).
