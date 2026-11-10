# Journal de Développement - Architecture et Décisions Techniques (Prysma)

## 1. Refonte de l'Arbre Syntaxique Abstrait (AST) et des Visiteurs

* **Injection de dépendances (Visiteur Général) :** Le passage initial du contexte par builder imposait des modifications répétées sur des dizaines d'emplacements pour une seule fonctionnalité, créant des risques d'objets non initialisés. L'injection directe du `ContextGenCode` dans le visiteur général centralise l'accès aux registres et à l'arène mémoire.
* **Récursion Mutuelle :** L'ancien algorithme d'extraction d'expressions, basé sur le balayage manuel de délimiteurs, manquait de robustesse lors de l'intégration d'appels de fonctions imbriqués dans des calculs arithmétiques. Son remplacement par une récursion mutuelle entre `BuilderTreeEquation` et `BuilderTreeInstruction` stabilise la résolution des expressions complexes.
* **Typage et Abstraction :** L'encapsulation de `llvm::Value*` derrière l'abstraction `IType*` au niveau des registres fournit au compilateur l'information sémantique exacte et l'adresse mémoire associée sans nécessiter d'inférence tardive.
* **Génération Automatique des Nœuds :** Les classes de nœuds AST et leurs visiteurs associés sont générés dynamiquement par des templates Jinja2 / Python à partir d'une description déclarative YAML (`ast.yaml`), garantissant la cohérence des énumérations de nœuds sans code répétitif manuel.
* **Suppression des `dynamic_cast` :** Remplacement des casts dynamiques au runtime par un système d'identifiants énumérés générés à la compilation, accélérant l'identification du type de nœud.

## 2. Gestion de la Mémoire et LLVM

* **Allocateur d'Arène (`llvm::BumpPtrAllocator`) :** La gestion de la mémoire de l'AST a migré de `std::shared_ptr` vers des arènes contiguës LLVM. L'utilisation de pointeurs bruts (`INode*`) au sein de l'arène optimise la localité de cache L1/L2 et élimine les fuites mémoire sans nécessiter de parcours de désallocation récursif.
* **Transmission par Pointeur Opaque :** Lors du passage de structures en mémoire, le compilateur transmet l'adresse mémoire directe sous forme de pointeur 64 bits (`ptr`), évitant les copies complètes de structures.
* **Tableaux par Composition :** L'architecture des tableaux s'appuie désormais sur la composition plutôt que sur l'héritage, autorisant des dimensions dynamiques pour le stockage des variables.
* **Décompilation LLVM 18 :** L'analyse du code machine LLVM intermédiaire via décompilation systématique de binaires C++ sert de base documentaire pour vérifier la conformité des structures de contrôle (`if-else`, boucles `while`, alignement des octets) avec les exigences de la version 18 de Clang.

## 3. Optimisations et Flux de Travail

* **Toolchain CMake, Clang 18 et LLD :** Configuration exclusive sous Clang 18 et l'éditeur de liens LLD avec prise en charge du cache `ccache` pour accélérer les compilations incrémentales. L'intégration de la découverte automatique de fichiers dans `CMakeLists.txt` simplifie l'arborescence des sources.
* **Parallélisme avec `llvm::ThreadPool` :** Remplacement des threads non bornés par le pool de threads de LLVM. La compilation multi-fichiers est scindée en deux passes : la Passe 1 (analyse lexicale, AST, signatures) et la Passe 2 (environnements locaux, émission LLVM IR), prévenant les conflits d'accès mémoire.
* **Synchronisation et Sécurité Thread-Safe :** Les registres globaux partagés sont protégés par mutex, et l'orchestrateur centralise la détection des dépendances circulaires d'inclusions via un ensemble de déduplication synchronisé.
* **Instrumentation Continue (ASAN / UBSan) :** AddressSanitizer et UndefinedBehaviorSanitizer sont injectés automatiquement lors de la compilation en mode débogage (`debug.py`).

## 4. Corrections Majeures Documentées

* **Résolution de l'Assignation Float :** L'erreur de segmentation lors de l'assignation entre représentations flottantes de précisions distinctes a été résolue par l'implémentation de conversions explicites (`creerAutoCast`).
* **Portée des Arguments dans les Appels Imbriqués :** L'appel imbriqué `call testArgDanIf(param - 1)` corrompait auparavant le registre partagé d'arguments. L'introduction d'une pile de contextes (`std::stack`) isole rigoureusement les variables locales de chaque frame d'activation.
* **Identifiants Visuels GraphViz :** Remplacement des identifiants statiques de nœuds par une pile d'identifiants incrémentale dans le visiteur GraphViz, éliminant les collisions de labels dans les arbres profonds.
* **Stockage des Retours de Fonctions dans des Variables :** Implémentation complète de la capture du résultat d'un appel direct : `dec <type> variable = call fonction(...)`.
* **Constantes Numériques en Arguments :** Correction du traitement des littéraux numériques bruts passés en paramètres (`call fonction(42, 100)`), assurant la parité de traitement entre variables et valeurs immédiates.

## 5. Algorithmique et Conception du Lexer

* **Tables Compile-Time (LUT) :** Classification de chaque octet ASCII en $O(1)$ sans saut conditionnel via une table de 256 entrées générée par métaprogrammation (`cclut_array`).
* **Résolution SWAR des Mots-Clés :** Encodage des mots-clés en entiers 64 bits à la compilation (`to_kw()`) et comparaison directe par lecture de blocs mémoire non alignés (`ld16`, `ld32`, `ld64`) sous `switch(word.size())`.
* **Résolution Unaire sans Défaut de Branchement :** Table booléenne précalculée (`unary_lut`) déterminant le rôle unaire ou binaire d'un opérateur arithmétique d'après le type de jeton précédent.

## 6. Prochaines Priorités d'Ingénierie

* **Parseur d'Expressions de Pratt :** Remplacement de la chaîne de responsabilité par un analyseur de Pratt pour linéariser le coût de construction des arbres d'équations.
* **Appels de Méthodes Chaînés :** Support de la syntaxe d'appels successifs sur valeurs retournées (`objet.getAutre().methode()`).
* **Déclarations Externes C (`extern`) :** Liaison native de fonctions de la bibliothèque standard C sans adaptateur intermédiaire.
