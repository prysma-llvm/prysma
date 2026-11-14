# Tests d’intégration de l’environnement de développement

Le point d’entrée principal reste `tests/run_unit_tests.py`. Les cinq étapes d’origine sont conservées.
Une sixième étape lance l’orchestrateur des tests de l’environnement de développement.

## Exécution

Python 3.12 est la version de référence en CI. Installez les dépendances de test sur la machine hôte, puis construisez l’image depuis la racine du dépôt :

```sh
python -m pip install -r requirements.txt -r tests/environnement_dev_tests/requirements.txt
docker build --target debug -t prysma-debug -f docker/compiler/Dockerfile .
python tests/run_unit_tests.py --dev-env
```

Sélection des tests :

```sh
python tests/run_unit_tests.py                              # les six étapes
python tests/run_unit_tests.py --existing                   # les cinq étapes d’origine
python tests/run_unit_tests.py --dev-env --family common
python tests/run_unit_tests.py --dev-env --family linux
python tests/run_unit_tests.py --dev-env --family window
python tests/run_unit_tests.py --dev-env --family vscode
python tests/run_unit_tests.py --dev-env --family window --native-only
```

Répétez `--family` pour sélectionner plusieurs familles. Une famille sélectionnée échoue si un composant requis est absent.
L’option `--native-only` lance une vérification partielle qui exclut Docker et VS Code ; elle ne valide donc pas ces composants.
Les tests propres à un autre système d’exploitation ne s’appliquent pas. Les autres familles sélectionnées continuent de s’exécuter après un échec.

## Familles et prérequis

| Répertoire | Composants réellement testés |
| --- | --- |
| `common/integration/` | Outils de l’image Docker, volumes, UID/GID, montages en lecture seule, absences et succès du cache ccache, recompilations, erreurs, journaux, protocole DAP de debugpy et débogage distant LLDB |
| `linux/integration/` | Scripts de lancement Bash, ports occupés, doubles lancements, interruption et relance, chemins avec espaces et accents, base de compilation CMake et protocole LSP de clangd |
| `window/integration/` | Analyse syntaxique PowerShell native et gestion des cibles absentes ; montages Docker Desktop et codes de sortie lorsque les tests Docker sont sélectionnés |
| `vscode/` | Véritable instance VS Code exécutant les tests d’extension, points d’arrêt Python et C++ de Prysma, tâches avant et après le débogage, échec de compilation avec un binaire existant |
| `fixtures/` | Programmes déterministes utilisés par les tests des débogueurs |

Les clients de protocole partagés sont de petits utilitaires de test. Les services Docker, debugpy et LLDB ne sont pas simulés.
Le scénario clangd génère et compile un véritable projet CMake dans Docker, adapte sa base de compilation, puis vérifie sur la machine hôte la navigation vers une définition, les informations au survol, la complétion et la mise à jour des diagnostics avec un en-tête LLVM.

Sous Linux, Docker Engine, Bash, Python, GNU timeout et clangd sur la machine hôte sont requis. Les tests Docker sous Windows nécessitent Python natif, PowerShell et Docker Desktop utilisant des conteneurs Linux. Exécuter PowerShell sous Linux ne valide pas Windows.

Les tests VS Code nécessitent une session graphique (ou `xvfb-run` sous Linux), la commande `code` et un accès réseau pour installer des versions fixées des extensions de débogage dans un profil temporaire. Définissez `PRYSMA_VSCODE_BIN` si nécessaire. Ces tests copient les sources, compilent Prysma dans Docker avec deux tâches en parallèle et introduisent volontairement une erreur dans cette copie uniquement. Ils utilisent la même API de débogage que F5 ; les entrées clavier et le rendu visuel de l’interface ne sont pas vérifiés.

## Isolation, délais et résultats

Les conteneurs et volumes de test utilisent des noms uniques et une étiquette `prysma.test`. Les scripts de débogage déterminent leurs noms de conteneurs à partir de l’espace de travail et refusent les conflits. Seuls les conteneurs appartenant à la session sont supprimés. Les ports sont liés à l’interface de boucle locale. La vérification de disponibilité de LLDB examine le socket d’écoute du conteneur sans consommer sa connexion de débogage ; les tests vérifient aussi les échanges du protocole.

Le lancement Python utilise une tâche VS Code en arrière-plan, comme LLDB. Les scripts restent actifs jusqu’à l’arrêt du conteneur de débogage ; `--stop` arrête uniquement la session de cet espace de travail. Les deux scripts suppriment leur propre conteneur à la fin ou lors d’une interruption.

Les tâches `Prepare ... Debugging`, exécutées au premier plan, enchaînent la compilation et le démarrage du serveur. Un échec de compilation peut ainsi interrompre le lancement avant que VS Code attende le signal de disponibilité du serveur en arrière-plan. Le paramètre `debug.onTaskErrors` vaut `abort` dans l’espace de travail.

Configuration :

| Paramètre | Valeur par défaut |
| --- | --- |
| `PRYSMA_TEST_IMAGE` | `prysma-debug` |
| `PRYSMA_TEST_COMMAND_TIMEOUT` | 90 secondes par commande Docker des tests |
| `--family-timeout` | 1800 secondes par famille |
| `PRYSMA_DEBUG_START_TIMEOUT` | 15 secondes pour qu’un serveur commence à écouter |
| `PRYSMA_DOCKER_TIMEOUT` | 30 secondes par opération Docker des scripts Bash |
| `PRYSMA_DEBUG_IMAGE` / `PRYSMA_DEBUG_PORT` | Image de débogage et port du débogueur par défaut |
| `PRYSMA_BUILD_JOBS` | Nombre de processeurs pour les compilations habituelles ; les tests utilisent 2 |

Une session de développement suspendue sur un point d’arrêt n’a pas de délai d’expiration. Chaque famille de tests automatisés possède son propre délai maximal.
Les résultats sont enregistrés dans `build/dev-env-results/<run-id>/` : un journal et un fichier XML JUnit par famille, ainsi qu’un fichier `summary.json` contenant les informations système, les versions des composants, l’identité de l’image, la sélection des tests, les durées et les codes de sortie.
Utilisez `--results-dir PATH` pour changer cet emplacement. Pytest capture les sorties des sous-processus pour faciliter le diagnostic.

## CI Windows

Les parties WSL2 et virtualisation imbriquée du workflow GitHub Actions Windows sont **expérimentales**.
Le workflow utilise des machines `windows-2022` hébergées par GitHub et **Docker Engine dans une distribution Ubuntu 24.04 importée sous WSL2**, plutôt que Docker Desktop.

Les tests PowerShell natifs s’exécutent dans une tâche CI distincte, même si la préparation de WSL échoue. La tâche WSL tente l’installation, importe et démarre WSL2, installe Docker Engine et vérifie l’exécution d’un véritable conteneur avant de construire l’image de développement. Un échec de préparation fait échouer cette tâche ; il ne compte pas comme une exécution de tests réussie ou ignorée. Les tests démarrent seulement après une préparation réussie.

La tâche WSL exécute les familles `common`, `linux` et `vscode`, avec de véritables points d’arrêt Python et C++ de Prysma, ainsi que le refus de démarrer après une compilation échouée lorsqu’un binaire existe déjà. VS Code 1.96.4 fonctionne sous Xvfb avec un utilisateur Linux ordinaire ayant accès à Docker. La copie de travail est placée dans le système de fichiers Linux de la distribution pour conserver le comportement Linux des propriétaires et permissions d’exécution. Le script d’installation est destiné uniquement à la distribution neuve de la CI, pas à la machine d’un développeur.

Les tests WSL valident le comportement Linux dans WSL, y compris l’application VS Code pour Linux. Ce workflow ne valide **pas** VS Code Windows avec Remote WSL, le débogage Windows natif avec Docker Desktop, un espace de travail situé hors du lecteur C: ou F5 dans VS Code Windows natif. Pour couvrir Windows natif, exécutez les familles complètes `window` et `vscode` sur une machine Windows équipée de Docker Desktop.

Le workflow s’exécute lors des push et des pull requests. Il peut aussi être lancé depuis **Actions → CI Windows → Run workflow** une fois présent sur la branche par défaut. Les deux tâches doivent réussir : `windows-native` vérifie PowerShell natif et `wsl-docker` vérifie l’environnement WSL. Téléchargez les artefacts `windows-native-results` et `wsl-docker-results` de l’exécution pour consulter les journaux, les résultats JUnit, `summary.json` et `vscode-scenarios.json`. Les diagnostics WSL sont récupérés même après un échec des tests ; les échecs antérieurs au démarrage de WSL restent visibles dans le journal de l’étape de préparation. La distribution dédiée est arrêtée à la fin.

La préparation WSL suit la [référence des commandes Microsoft](https://learn.microsoft.com/fr-fr/windows/wsl/basic-commands). L’instance VS Code utilise le [point d’entrée documenté pour les tests d’extension](https://code.visualstudio.com/api/working-with-extensions/testing-extension).

## Limites de couverture restantes

Le plan de test fourni est plus large que cette suite. Ne sont pas encore couverts : tous les cas particuliers des points d’arrêt (fichiers portant le même nom, sources obsolètes, binaires optimisés ou sans symboles), toutes les représentations de variables, les copies partielles d’en-têtes LLVM, le déplacement du projet, toutes les combinaisons de diagnostics et d’annulation des tâches VS Code, l’arrêt brutal de l’éditeur et le fonctionnement de bout en bout de l’interface Flatpak. Le script Linux d’accès à Docker depuis Flatpak reste pris en charge, mais les tests exécutés sur la machine hôte ne prouvent pas une validation complète de l’interface Flatpak.

Les guides manuels Linux et Windows complètent les tests automatisés ; ils ne sont pas comptabilisés comme des réussites automatisées.
