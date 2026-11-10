# Architecture du Compilateur Prysma

## Vue d'ensemble

Prysma est un compilateur système ciblant l'infrastructure LLVM 18. Son architecture est découpée en phases indépendantes : analyse lexicale par tables de correspondance précalculées à la compilation, analyse syntaxique descendante récursive, validation sémantique et résolution de symboles en deux passes, puis émission de code intermédiaire LLVM IR en forme SSA (Single Static Assignment).

## Schéma d'exécution du compilateur

```
Fichier Source Prysma (.p)
           ↓
     [FileReading] - Chargement du tampon mémoire
           ↓
  [Lexer::tokenize] - Tables LUT compile-time + SWAR (O(n))
           ↓
    Vecteur de Tokens
           ↓
[BuilderTreeInstruction::build] - Descente récursive (Passe 1)
           ↓
  [BuilderTreeEquation] - Résolution d'expressions
           ↓
   Arbre Syntaxique Abstrait (AST alloué en arène BumpPtr)
           ↓
[FillingVisitorRegistry & FillingVisitorBodyClass] - Inspection des déclarations
           ↓
[BuilderEnvironmentRegistryFunction & Variable] - Tables de symboles (Passe 2)
           ↓
  [GeneralVisitorGenCode] - Émission LLVM IR (.ll)
           ↓
 [LLVM Backend / LLD] - Optimisations & Génération binaire machine
           ↓
    Binaire Exécutable Natif
```

## Frontend et Pipeline de Compilation

### 1. Lecture de fichier (`FileReading`)

Le composant `FileReading` extrait le contenu brut du fichier source `.p` à partir de son chemin absolu ou relatif résolu et charge le tampon textuel en mémoire.

### 2. Analyse lexicale (`Lexer::tokenize`)

La méthode `Lexer::tokenize(const std::string& sourceCode)` convertit le flux de caractères en un vecteur de structures `Token` :

* **Classification des caractères (LUT 256 octets) :** Une table précalculée à la compilation (`cclut_array`), générée par métaprogrammation avec `std::index_sequence` (`make_cclut`), classifie chaque octet ASCII (`CharClass`) en $O(1)$ sans saut de branchement conditionnel.
* **Chargement mémoire rapide (SWAR) :** Des fonctions de lecture directes (`ld16`, `ld32`, `ld64`) lisent des blocs de 2, 4 ou 8 octets directement dans les registres du processeur via `std::memcpy`.
* **Encodage compile-time des mots-clés (`to_kw`) :** La fonction `constexpr Lexer::to_kw()` transforme jusqu'à 8 caractères ASCII en un entier 64 bits en Little Endian (`val |= (c << (8 * i))`).
* **Résolution des mots-clés sans table de hachage :** `Lexer::resolveKeyword` effectue un `switch(word.size())` sur la longueur du token et compare directement l'entier 64 bits en un cycle processeur.
* **Résolution d'opérateurs unaires (`unary_lut`) :** Une table booléenne compile-time de 128 entrées détermine en $O(1)$ si un opérateur (`+`, `-`) se situe dans un contexte unaire ou binaire selon la nature du token précédent, éliminant les défauts de prédiction de branchement (*branch mispredictions*).
* **Pré-allocation mémoire :** Le vecteur de tokens pré-alloue sa mémoire via `tokens.reserve((sourceCode.size() / 3) + 4)` pour limiter les réallocations dynamiques sur le tas.
* **Fin de fichier déterministe :** Un token terminal `TOKEN_EOF` est systématiquement inséré en fin de flux.

### 3. Construction de l'Arbre Syntaxique Abstrait (AST)

La méthode `BuilderTreeInstruction::build(std::vector<Token>& tokens)` analyse la structure séquentielle du programme :

* **Descente récursive :** Chaque instruction consomme ses tokens et délègue l'instanciation des nœuds aux analyseurs spécialisés enregistrés dans `RegistryInstruction` (`ParserVariable`, `ParserIf`, `ParserWhile`, `ParserClass`, etc.).
* **Expressions et priorité (`BuilderTreeEquation`) :** La résolution des expressions arithmétiques et logiques s'appuie sur une chaîne de priorité (`ChainOfResponsibility`) qui localise l'opérateur de plus faible priorité pour découper l'expression en sous-arbres gauche et droit.

### 4. Pipeline en Deux Passes (`UnitCompilation`)

La compilation de chaque unité de traduction est orchestrée en deux passes explicites afin de résoudre les dépendances circulaires et l'ordre arbitraire de déclaration des fonctions et classes :

#### Passe 1 (`UnitCompilation::pass1`)
1. Résolution du chemin absolu et construction d'un nom d'unité unique (`<dossier_parent>_<nom_fichier>`).
2. Tokenisation du code source via `Lexer::tokenize`.
3. Construction de l'AST via `builderTreeInstruction->build(tokens)`.
4. Visite de l'arbre par `FillingVisitorRegistry` pour enregistrer les signatures globales de fonctions dans `RegistryFunctionGlobal`.
5. Visite de l'arbre par `FillingVisitorBodyClass` pour inspecter les définitions de classes, leurs attributs et leurs méthodes.

#### Passe 2 (`UnitCompilation::pass2`)
1. Instanciation et remplissage des environnements locaux de variables et de fonctions via `BuilderEnvironmentRegistryFunction` et `BuilderEnvironmentRegistryVariable`.
2. Parcours de l'AST par `GeneralVisitorGenCode` pour traduire les nœuds en instructions LLVM IR conformes au modèle SSA.
3. Export optionnel de l'arbre au format DOT / SVG via `GeneralVisitorGraphViz` si l'option `--graphviz` est activée.
4. Écriture du module LLVM résultant (`.ll`) sur le disque via `LlvmSerializer`.

## Gestion de la Mémoire Interne

### Allocateur d'Arène (`llvm::BumpPtrAllocator`)

Pour garantir une localité de cache optimale et éliminer la surcharge des allocations individuelles :

* Les nœuds AST (`INode*`) sont alloués directement au sein d'une arène contiguë (`llvm::BumpPtrAllocator`).
* L'allocation se résume à l'incrémentation d'un pointeur de sommet (*bump pointer*), avec une complexité en $O(1)$.
* Les pointeurs intelligents (`std::shared_ptr`) sont exclus du parcours critique de l'AST.
* La mémoire complète de l'AST d'une unité de traduction est libérée en bloc à la destruction du contexte de compilation, prévenant toute fuite mémoire sans coût de parcours récursif de désallocation.

## Compilation Multi-Fichiers et Concurrence

La compilation de projets multi-fichiers est coordonnée par `OrchestratorInclude` :

* **Pool de threads (`llvm::ThreadPool`) :** Parallélisation native de la Passe 1 puis de la Passe 2, bornée aux capacités du processeur pour éviter l'épuisement des ressources système.
* **Résolution des inclusions (`include`) :** Lorsqu'une directive `include "chemin/fichier.p";` est rencontrée, `OrchestratorInclude::includeFile()` enregistre le chemin résolu.
* **Prévention des inclusions multiples et cycles :** Un ensemble `std::set<std::string> _alreadyIncludedFiles` protégé par un mutex garantit qu'un même fichier n'est compilé qu'une seule fois.
* **Gestion des erreurs et Fail-Fast :** Les exceptions `CompilationError` levées dans un thread worker sont capturées, consignées avec le fichier et la ligne fautive, et mettent à jour le témoin atomique `_hasErrors`. Si la Passe 1 échoue sur un fichier quelconque, la Passe 2 est automatiquement annulée.

## Formats et Sorties du Compilateur

* **Fichier source :** `.p`
* **Code intermédiaire :** `.ll` (LLVM IR textuel) stocké dans le répertoire `programme/`
* **Visualisation graphique optionnelle :** `.dot` et `.svg` stockés dans le répertoire `graphe/`
* **Binaire exécutable natif :** Généré via Clang / LLD à partir des modules `.ll`.

## Perspectives d'Évolution Technique

1. **Parseur d'expressions de Pratt :** Remplacement de la chaîne de responsabilité par un analyseur syntaxique de Pratt pour réduire la complexité de résolution d'expressions de $O(k \cdot e^2)$ à $O(e)$.
2. **Dispatch statique des nœuds AST :** Remplacement progressif du double dispatch virtuel (`accept` / `visit`) par un dispatch statique basé sur les identifiants de nœuds contigus, optimisant la prédiction de branchement lors de l'émission LLVM.
3. **Internement de chaînes (String Interning Pool) :** Centralisation des identifiants dans une table de chaînes uniques permettant la comparaison par égalité de pointeurs plutôt que par comparaison lexicographique.
