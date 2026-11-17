# Prysma Programming Language

![Prysma Logo](img/prysma.svg)

[![CI](https://github.com/Zyphorah/Prysma/actions/workflows/ci.yml/badge.svg)](https://github.com/Zyphorah/Prysma/actions/workflows/ci.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![C++](https://img.shields.io/badge/C++-23-blue.svg)](https://isocpp.org/)
[![LLVM](https://img.shields.io/badge/LLVM-18-red.svg)](https://llvm.org/)

Prysma est un langage de programmation système compilé vers l'infrastructure **LLVM 18**. Le projet se concentre sur le contrôle déterministe des ressources sans ramasse-miettes (garbage collector), une tokenisation optimisée par tables précalculées à la compilation, l'allocation contiguë par arènes et la compilation multi-fichiers parallélisée.

## Capacités Techniques

* **Backend LLVM :** Génération de code intermédiaire LLVM 18 en forme SSA (Single Static Assignment) exploitant les pointeurs opaques.
* **Analyse Lexicale :** Tokenisation par tables de correspondance précalculées à la compilation (LUT 256 octets) et comparaisons d'entiers 64 bits en Little Endian, évitant les surcoûts de hachage dynamique.
* **Gestion Mémoire :** Gestion manuelle sur le tas via les opérateurs `new` et `delete`. Allocation interne des nœuds AST optimisée par arène (`llvm::BumpPtrAllocator`).
* **Compilation Multi-fichiers :** Pipeline en 2 passes parallélisé avec `llvm::ThreadPool`, résolution des directives `include` et détection des cycles de dépendances.
* **Modèle Objet :** Classes avec visibilité `public:`, `private:`, `protected:`, héritage et polymorphisme dynamique via table virtuelle (vtable).
* **Analyse et Typage :** Typage statique avec auto-casting sécurisé, pile de contextes pour les portées lexicales et détection précoce des erreurs syntaxiques (fail-fast).

## Architecture du Compilateur

Le compilateur opère selon un pipeline modulaire :

1. **Frontend :** Analyse lexicale sans saut de branchement conditionnel (`Lexer::tokenize`) et descente récursive pour les instructions.
2. **Passe 1 (Analyse & Registres) :** Construction de l'AST et inspection des signatures via `FillingVisitorRegistry` et `FillingVisitorBodyClass`.
3. **Passe 2 (Environnements & Émission) :** Résolution des tables de symboles (`BuilderEnvironmentRegistryFunction`, `BuilderEnvironmentRegistryVariable`) et émission LLVM IR via `GeneralVisitorGenCode`.
4. **Backend LLVM :** Optimisations du module et liaison binaire par l'infrastructure LLVM / LLD.

## Syntaxe

```rust
fn int32 main() {
    dec string message = "Système Prysma opérationnel";
    call print(ref message);
    return 0;
}
```

## Compilation sous Docker

La chaîne de compilation officielle (LLVM 18, Clang 18, LLD, CMake, ccache, Python) est encapsulée dans une image Docker pour partager la même chaîne d’outils. Des reconstructions peuvent sélectionner des versions de paquets différentes ; le digest de l’image identifie la version effectivement utilisée.

### Installation de Docker

| Système hôte | Guide d'installation |
| :--- | :--- |
| **Arch Linux** | [Guide Arch Linux](docs/fr/installation/docker/arch_linux.md) |
| **Debian** | [Guide Debian](docs/fr/installation/docker/debian.md) |
| **Ubuntu** | [Guide Ubuntu](docs/fr/installation/docker/ubuntu.md) |
| **Windows (PowerShell)** | [Guide Windows](docs/fr/installation/docker/windows_powershell.md) |

### Démarrage rapide

1. **Construire l'image de compilation :**
   ```bash
   docker build --target debug -t prysma-debug -f docker/compiler/Dockerfile .
   ```

2. **Compiler le projet depuis l'hôte :**
   ```bash
   # Mode Release optimisé (-O3, -march=native, -flto, lld)
   docker run --rm -v "$(pwd)":/workspace prysma-debug python3 build.py

   # Mode Debug instrumenté (-g3, -O0, ASan, UBSan)
   docker run --rm -v "$(pwd)":/workspace prysma-debug python3 debug.py
   ```

3. **Session interactive :**
   ```bash
   docker run --rm -it -v "$(pwd)":/workspace prysma-debug
   ```

Le script `build.py` / `debug.py` génère automatiquement les classes de nœuds AST, les interfaces des visiteurs et les tables du parseur avant d'invoquer CMake et Clang.

### Intégration VS Code

La configuration fournie dans `.vscode` s'interface directement avec le conteneur :

* **Compilation (`Ctrl+Shift+B`) :** Compile le fichier source `.p` actif via `docker/environnement_dev/linux/docker-cli.sh` (ou `docker/environnement_dev/windows/docker-cli.ps1` sous Windows).
* **Débogage (`F5`) :** Profils LLDB pour le compilateur C++ (port 1234) et debugpy pour les générateurs Python (port 5678).
* **Indexation Clangd :** Navigation de code et autocomplétion C++ sur l'hôte sans nécessiter LLVM en local.
* **Coloration syntaxique :** Extension `.vscode/SyntaxeCouleurPrysma` pour les fichiers `.p`.

Voir le [Guide de l'environnement VS Code](docs/fr/installation/vscode_environment.md) pour les détails de configuration. Pour les options de compilation avancées, voir le [Guide de compilation Docker](docs/fr/installation/docker_compilation.md).

## Documentation

La documentation technique est disponible dans le répertoire `docs/` :

* **Compilation et Environnement :**
  * [Architecture de centralisation et guide Docker](docs/fr/installation/docker_compilation.md)
  * [Environnement de travail et débogage sous VS Code](docs/fr/installation/vscode_environment.md)
  * [Écrire des tests d’intégration Prysma](docs/fr/integration_tests.md)
  * Guides Docker : [Arch Linux](docs/fr/installation/docker/arch_linux.md) • [Debian](docs/fr/installation/docker/debian.md) • [Ubuntu](docs/fr/installation/docker/ubuntu.md) • [Windows](docs/fr/installation/docker/windows_powershell.md)
  * [Configuration native locale Ubuntu (legacy)](docs/fr/installation/setup_ubuntu.md)
* **Architecture & Conception :**
  * [Analyse de l'Architecture](docs/fr/architecture.md)
  * [Analyse de Robustesse et Sécurité](docs/fr/robustness_security.md)
  * [Spécifications du Langage Prysma](docs/fr/language_specification.md)
  * [Rapport de choix technologiques](docs/fr/analysis/technology_choices.md)

## Licence

Ce projet est distribué sous licence **GPL v3**. Consulter le fichier `LICENSE` pour les détails juridiques.

## Mainteneurs officiels

* Raphael Arseneault ([@Zyphorah](https://github.com/Zyphorah))
* Félix-Olivier Dumas ([@unrays](https://github.com/unrays))
