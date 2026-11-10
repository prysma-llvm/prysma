# Prysma Programming Language

![Prysma Logo](img/prysma.svg)

[![CI](https://github.com/Zyphorah/Prysma/actions/workflows/ci.yml/badge.svg)](https://github.com/Zyphorah/Prysma/actions/workflows/ci.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![LLVM](https://img.shields.io/badge/LLVM-18-red.svg)](https://llvm.org/)

Prysma est un langage de programmation système à hautes performances, compilé vers l'infrastructure **LLVM 18**. Le projet privilégie un contrôle déterministe des ressources et une architecture modulaire robuste.

## Capacités Techniques

  * **Backend :** Génération de code intermédiaire (IR) via LLVM 18 en forme SSA (Single Static Assignment).
  * **Modèle Objet :** Implémentation native des classes, de l'héritage et du polymorphisme dynamique via VTable.
  * **Gestion Mémoire :** Contrôle manuel du tas (heap) via les opérateurs `new` et `delete`. Allocation interne optimisée par **Bump Pointer Allocator** (Arena).
  * **Parallélisme :** Orchestration de la compilation multi-fichier via `llvm::ThreadPool`.
  * **Analyse Statique :** Système de typage fort avec auto-casting sécurisé et détection des dépendances circulaires d'inclusion.

## Architecture du Compilateur

Le pipeline de traitement suit une structure linéaire stricte :

1.  **Frontend :** Analyse lexicale (Lexer) et syntaxique par descente récursive.
2.  **Représentation Intermédiaire :** Construction d'un Arbre Syntaxique Abstrait (AST) basé sur le patron de conception **Composite**.
3.  **Génération de Code :** Traduction de l'AST vers LLVM IR via le patron de conception **Visitor**.
4.  **Backend :** Optimisations et génération du binaire natif par l'infrastructure LLVM.

## Aperçu de la Syntaxe

```rust
fn int32 main() {
    dec string message = "Système Prysma opérationnel";
    call print(ref message);
    return 0;
}
```

## Programmation orientée objet

> Actuellement en phase expérimentale. Cette partie est incomplète et peut présenter des instabilités.

## Environnement de compilation officiel (Docker)

La compilation officielle de Prysma repose sur une **architecture centralisée sous Docker**. 

Cette approche standardise l'ensemble de la chaîne d'outils (LLVM 18, Clang 18, LLD, CMake, ccache, outils de diagnostic et environnement Python) dans une image reproductible. Elle élimine les disparités entre distributions hôtes et évite toute pollution du système local.

### Installation de Docker

Avant de compiler, assurez-vous que Docker est opérationnel sur votre machine :

| Système hôte | Guide d'installation |
| :--- | :--- |
| **Arch Linux** | [Consulter le guide Arch Linux](docs/fr/installation/docker/arch_linux.md) |
| **Debian** | [Consulter le guide Debian](docs/fr/installation/docker/debian.md) |
| **Ubuntu** | [Consulter le guide Ubuntu](docs/fr/installation/docker/ubuntu.md) |
| **Windows (PowerShell)** | [Consulter le guide Windows](docs/fr/installation/docker/windows_powershell.md) |

### Démarrage rapide

Cet environnement constitue l'espace de travail officiel du projet. Il s'adresse aussi bien à ceux qui souhaitent simplement compiler et tester Prysma qu'à ceux qui désirent y contribuer activement. 

Une configuration de base est fournie pour faciliter le développement sous **VS Code** (VS Code standard / « bleu ») via le dossier `.vscode` et le script `docker-cli.sh`. Elle permet notamment de compiler directement avec `Ctrl+Shift+B` le fichier Prysma actif dans l'éditeur (réservé exclusivement à la compilation des fichiers avec l'extension `.p`), tout en prenant en charge le débogage et l'indexation de code Clangd sans nécessiter d'outils installés sur l'hôte.

#### 1. Construire l'image de compilation :

```bash
docker build --target debug -t prysma-debug -f docker/compiler/Dockerfile .
```

#### 2. Accéder à l'environnement interactif :

```bash
docker run --rm -it -v "$(pwd)":/workspace prysma-debug
```

#### 3. Orchestration de la compilation (`build.py` et `debug.py`) :

Une fois dans le conteneur (dans `/workspace`), la compilation est orchestrée par deux scripts Python qui servent d'interface unifiée :

* **Pourquoi un wrapper Python ?** Avant de lancer CMake et Clang, le script exécute automatiquement les générateurs de code internes du compilateur (construction de l'AST, des visiteurs, de l'export GraphViz et du parseur), active le cache `ccache` et configure les options de compilation sans avoir à saisir de longues commandes manuelles.
* **`python3 build.py` (Mode Release)** : Génère le code et compile une version hautement optimisée (`-O3`, `-march=native`, `-flto`, éditeur de liens `lld`).
* **`python3 debug.py` (Mode Debug)** : Génère le code et compile avec l'injection des drapeaux de débogage et d'analyse (`-g3`, `-O0`, protecteur de pile `-fstack-protector-all`, assertions, ainsi que les détecteurs de corruption mémoire AddressSanitizer `-fsanitize=address` et UndefinedBehaviorSanitizer `-fsanitize=undefined`).

*(Il est également possible de compiler directement depuis l'hôte sans ouvrir de shell interactif, par exemple : `docker run --rm -v "$(pwd)":/workspace prysma-debug python3 build.py`).*

### Environnement de travail et débogage sous VS Code

Le projet propose une intégration complète prête à l'emploi sous **VS Code** (VS Code standard / « bleu ») alliant le confort de l'interface graphique hôte et la puissance du conteneur Docker :

* **Profil préconfiguré (`.vscode/Prysma.code-profile`)** : Installe et paramètre en un clic l'ensemble des extensions (Clangd, LLDB DAP, Debugpy) et réglages recommandés.
* **Coloration syntaxique Prysma (`.vscode/SyntaxeCouleurPrysma`)** : Extension interne pour la coloration et la gestion des fichiers sources `.p`.
* **Compilation à la volée** : Le raccourci `Ctrl+Shift+B` compile immédiatement le fichier `.p` actif via l'exécutable Prysma dans le conteneur.
* **Débogage pas-à-pas (C++ et Python)** : Configurations prêtes à l'emploi (`F5`) pour déboguer le compilateur natif via LLDB (port 1234), les tests unitaires C++ et les scripts Python via debugpy (port 5678).
* **Navigation et autocomplétion Clangd** : Analyse sémantique en temps réel sans installer LLVM sur votre système hôte.

Consultez le [Guide de l'environnement VS Code](docs/fr/installation/vscode_environment.md) pour les instructions pas à pas d'installation du profil, de packaging de l'extension de coloration et de débogage.

Pour les options avancées de compilation et les benchmarks, consultez également le [Guide d'architecture et de compilation Docker](docs/fr/installation/docker_compilation.md).

## Documentation

L'intégralité de la documentation technique et des procédures de configuration est organisée dans le répertoire `/docs` :

* **Compilation et Environnement :**
  * [Architecture de centralisation et guide Docker](docs/fr/installation/docker_compilation.md)
  * [Environnement de travail et débogage sous VS Code](docs/fr/installation/vscode_environment.md)
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
