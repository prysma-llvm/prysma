# Guide de l'environnement de développement et débogage sous VS Code

Ce document détaille la configuration de l'environnement de travail sous VS Code pour développer, déboguer et tester le projet Prysma avec l'environnement Docker.

## 1. Vue d'ensemble de l'environnement

L'architecture combine l'éditeur VS Code sur votre machine hôte et l'isolation du conteneur de compilation :

* **Aucune dépendance système lourde sur l'hôte** : LLVM 18, Clang 18, LLD, GDB et LLDB tournent exclusivement dans le conteneur `prysma-debug`.
* **Indexation C++ avec Clangd** : Le wrapper `docker/environnement_dev/linux/docker-cli.sh` (ou `docker/environnement_dev/windows/docker-cli.ps1`) réécrit automatiquement les chemins dans `compile_commands.json` pour que Clangd sur l'hôte fournisse l'autocomplétion et la navigation de code sans nécessiter LLVM en local.
* **Débogage complet (C++ et Python)** : Des profils de lancement permettent de poser des points d'arrêt et d'inspecter l'exécution dans le conteneur via LLDB et debugpy.
* **Coloration syntaxique Prysma** : Une extension interne gère les fichiers sources `.p`.

## 2. Installation du profil officiel (`.vscode/Prysma.code-profile`)

Le fichier `.vscode/Prysma.code-profile` regroupe les extensions recommandées, les règles de formatage et les réglages du débogueur.

### Procédure d'importation dans VS Code

1. Ouvrez VS Code dans le dossier du projet `prysma`.
2. Ouvrez la palette de commandes avec `Ctrl+Shift+P` et tapez `Profiles: Import Profile` (ou via le menu *Paramètres (engrenage) > Profils > Importer le profil...*).
3. Choisissez **Sélectionner un fichier...** (`Select File...`) et sélectionnez le fichier :  
   `.vscode/Prysma.code-profile`
4. Cochez les éléments à importer (Paramètres, Extensions, Raccourcis) et cliquez sur **Créer le profil** (ou **Importer**).

Une fois le profil appliqué, VS Code installe automatiquement les extensions recommandées (`llvm-vs-code-extensions.vscode-clangd`, `llvm-vs-code-extensions.lldb-dap`, `ms-python.debugpy`, etc.) et désactive l'IntelliSense C++ par défaut au profit de Clangd.

## 3. Installation de la coloration syntaxique Prysma (`SyntaxeCouleurPrysma`)

Le dossier `.vscode/SyntaxeCouleurPrysma` contient le code source de l'extension assurant la coloration syntaxique, les paires de délimiteurs et la grammaire TextMate pour les fichiers `.p`.

### Étape 1 : Empaqueter l'extension en fichier `.vsix`

Depuis votre terminal sur la machine hôte :

```bash
cd .vscode/SyntaxeCouleurPrysma
npm install
npx vsce package
```

Cette commande génère le fichier `prysma-language-support-0.0.2.vsix`.

> **Note :** Si Node.js / npm n'est pas installé sur votre hôte, vous pouvez exécuter cette commande dans n'importe quel environnement disposant de Node 18+.

### Étape 2 : Installer le `.vsix` dans VS Code

1. Dans l'explorateur de fichiers de VS Code, accédez au dossier `.vscode/SyntaxeCouleurPrysma`.
2. Faites un clic droit sur le fichier `.vsix` généré (`prysma-language-support-*.vsix`).
3. Cliquez sur **Install Extension VSIX** dans le menu contextuel (ou via la vue Extensions `Ctrl+Shift+X` > menu `...` en haut à droite > **Installer depuis le fichier VSIX...**).
4. Rechargez la fenêtre si demandé (`Ctrl+Shift+P` > `Developer: Reload Window`).

Les fichiers `.p` sont désormais automatiquement reconnus et colorés.

## 4. Tâches de compilation intégrées (`tasks.json`)

Le fichier `.vscode/tasks.json` fournit des tâches automatisées exécutées via le wrapper `docker/environnement_dev/linux/docker-cli.sh` (ou `docker/environnement_dev/windows/docker-cli.ps1` sous Windows) :

* **Compiler le fichier Prysma actif (`Ctrl+Shift+B`)** :  
  La tâche par défaut `Compile Prysma Program` compile directement le fichier source `.p` actuellement ouvert dans l'éditeur en invoquant l'exécutable `./build/Prysma` à l'intérieur du conteneur. Les erreurs et avertissements éventuels sont capturés par le regex `problemMatcher` et soulignés dans l'éditeur.
* **Recompiler Prysma (`Build`)** :  
  Exécute `debug.py` dans le conteneur Docker pour régénérer le code et reconstruire le compilateur avec les symboles de débogage et sanitizers.
* **Compiler les tests unitaires (`Build Unit Tests`)** :  
  Compile la suite de tests unitaires C++ sous `tests/unit_tests/build`.

## 5. Configurations de débogage (`launch.json`)

Le panneau d'exécution et débogage (`Ctrl+Shift+D` ou touche `F5`) propose trois configurations principales :

### Débogage du compilateur C++ (`C++: Debug Prysma in Docker (LLDB)`)
Cette configuration permet de déboguer le compilateur Prysma lui-même avec LLDB.  
Elle démarre automatiquement la tâche `Start LLDB Server (Docker)` qui lance `lldb-server-18` dans le conteneur sur le port `1234`. VS Code s'attache à distance (`gdb-remote 127.0.0.1:1234`) avec un mappage de sources `/workspace` vers votre dossier local `${workspaceFolder}`. Vous pouvez poser des points d'arrêt et inspecter les variables directement dans l'interface de VS Code.

### Débogage des tests unitaires (`C++: Debug Unit Tests in Docker (LLDB)`)
Recompile automatiquement les tests via `Build Unit Tests`, démarre `lldb-server` sur le binaire `PrysmaTests`, et attache le débogueur LLDB pour inspecter les échecs de tests.

### Débogage des scripts Python (`Python: Debug in Docker`)
Permet de déboguer les générateurs d'AST, le parseur ou les scripts d'orchestration (`debug.py` / `build.py`).  
La tâche `Start Python Debug Server (Docker)` lance `debugpy` dans le conteneur sur le port `5678` via `docker/environnement_dev/linux/docker-debug-python.sh`. VS Code s'y connecte avec mappage des chemins locaux vers `/workspace`. À la fermeture de la session, le conteneur s'arrête automatiquement.

## 6. Autocomplétion et navigation C++ (Clangd)

Le fichier `.vscode/settings.json` délègue l'analyse du code C++ à **Clangd** :
* `docker/environnement_dev/linux/docker-cli.sh` (ou `docker/environnement_dev/windows/docker-cli.ps1`) convertit automatiquement les chemins `/workspace` en chemins absolus locaux dans `build/compile_commands.json` après chaque build.
* Les en-têtes LLVM internes sont accessibles via le lien `.clangd-includes`.
* Clangd fournit l'analyse sémantique, la navigation vers les définitions (`F12`), les diagnostics Clang-Tidy et les indications d'inlay hints en temps réel.
