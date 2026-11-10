# Installation Native sur Ubuntu (Alternative Hors Conteneur)

> **Important :** Ce guide documente l'installation manuelle directe sur l'hôte Ubuntu. La méthode officielle et recommandée de compilation de Prysma repose sur l'architecture Docker centralisée : voir le [Guide de compilation Docker](docker_compilation.md) et le [Guide de l'environnement VS Code](vscode_environment.md).

## 1. Prérequis Système

Installez la chaîne d'outils LLVM 18, le compilateur Clang, l'éditeur de liens LLD et les dépendances d'analyse :

```bash
sudo apt update
sudo apt install -y llvm-18 llvm-18-dev clang-18 lld-18 bear graphviz cmake ninja-build ccache python3 python3-venv
```

## 2. Environnement Virtuel Python

Configurez l'environnement virtuel pour exécuter les générateurs de code internes du compilateur :

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## 3. Compilation du Projet

Pour compiler le projet en mode débogage avec AddressSanitizer :

```bash
python3 debug.py
```

Pour compiler le projet en mode optimisé :

```bash
python3 build.py
```

## 4. Configuration de VS Code (`tasks.json`)

Si vous développez hors Docker, la tâche de compilation à la volée (`Ctrl+Shift+B`) invoque directement le binaire compilé dans `build/` :

```json
{
  "label": "Compiler Prysma (Natif)",
  "type": "shell",
  "command": "./build/Prysma",
  "args": ["${file}"],
  "options": {
    "cwd": "${workspaceFolder}"
  },
  "group": {
    "kind": "build",
    "isDefault": true
  },
  "presentation": {
    "reveal": "silent",
    "panel": "shared",
    "clear": true,
    "focus": false
  },
  "problemMatcher": {
    "owner": "prysma_language",
    "fileLocation": ["absolute"],
    "pattern": {
      "regexp": "^([^:]+):(\\d+):(\\d+):\\s+(?:Error|error|warning):\\s+(.*)$",
      "file": 1,
      "line": 2,
      "column": 3,
      "message": 4
    }
  }
}
```

## 5. Coloration Syntaxique Prysma

Pour installer l'extension de coloration syntaxique locale depuis les sources :

```bash
cd .vscode/SyntaxeCouleurPrysma
npm install
npx vsce package
```

Puis installez le fichier `.vsix` généré via VS Code (*Extensions > menu `...` > Installer depuis le fichier VSIX...*).
