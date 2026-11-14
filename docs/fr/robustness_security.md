# Robustesse et Sécurité - Compilateur Prysma

## Vue d'ensemble

Ce document détaille les garanties architecturales, la gestion des erreurs et les mécanismes de synchronisation implémentés dans le compilateur Prysma pour traiter les cas limites syntaxiques, préserver l'intégrité de la mémoire et assurer la sûreté des exécutions concurrentes.

## 1. Analyse Lexicale : Sûreté d'Accès Mémoire (`Lexer::tokenize`)

**Composant :** `Lexer::tokenize`
**Rôle :** Premier point d'entrée pour les données brutes non fiables.

### Cas limite 1 : Fichier source vide ou composé uniquement d'espaces

* **Scénario :** Fichier `.p` de 0 octet ou contenant exclusivement des caractères d'espacement (`\t`, `\n`, `\r`, espaces).
* **Traitement :** La boucle de tokenisation atteint la fin du tampon sans émettre de symboles d'instruction et insère automatiquement un jeton unique `TOKEN_EOF`.
* **Résultat :** Le vecteur de tokens n'est jamais vide (taille minimale = 1). Le constructeur d'AST détecte immédiatement `TOKEN_EOF` et génère une unité minimale valide sans erreur de déréférencement ni comportement indéfini.

### Cas limite 2 : Chaîne textuelle non fermée

* **Scénario :**
  ```rust
  dec string message = "Chaîne interrompue avant la fin du fichier
  ```
* **Traitement :** Lors du traitement d'un guillemet ouvrant (`"`), l'analyseur valide à chaque itération la limite du tampon `cursor < end`. Si la fin du fichier est atteinte sans guillemet fermant, une exception `CompilationError` est immédiatement levée en indiquant le numéro de ligne.
* **Résultat :** Prévient tout dépassement de tampon en lecture (*buffer overread*) et bloque la progression vers la phase de parsing.

### Cas limite 3 : Fuzzing et caractères non-ASCII

* **Scénario :** Injection de séquences binaires aléatoires, octets corrompus ou encodages inattendus.
* **Traitement :** La table de classification précalculée de 256 octets (`cclut_array`) associe tout octet hors intervalle ASCII valide à la catégorie `CC_OTHER`. Le compilateur lève une erreur syntaxique explicite sans jamais provoquer d'avarie mémoire.
* **Vérification :** Une suite de fuzzing aléatoire (`Tester Lexer Fuzzing - Garbage aleatoire` dans `tests/unit_tests/src/compiler/lexer/test_lexer.cpp`) valide automatiquement la résilience du lexer face à 5000 octets aléatoires continus.

## 2. Analyse Syntaxique et Expressions (`BuilderTreeEquation`)

**Composant :** `BuilderTreeEquation` et `BuilderTreeInstruction`
**Rôle :** Construction de l'arbre syntaxique abstrait (AST) à partir du flux de tokens.

### Cas limite 1 : Parenthèses asymétriques ou vides

* **Scénario :**
  ```rust
  dec int32 x = ();
  dec int32 y = (10 + );
  dec int32 z = ((5 + 3);
  ```
* **Traitement :** `ServiceParenthese` vérifie l'intégrité et la symétrie des délimiteurs avant toute allocation de nœuds. Si une expression devient vide après élimination des parenthèses englobantes ou si une parenthèse fermante manque, une exception syntaxique est levée avec le numéro de ligne précis.
* **Résultat :** Aucun nœud incomplet n'est alloué dans l'arène mémoire.

### Cas limite 2 : Opérateurs consécutifs orphelins

* **Scénario :**
  ```rust
  dec int32 bad = 10 / / 2;
  ```
* **Traitement :** La chaîne de responsabilité (`ChainOfResponsibility`) isole les opérateurs orphelins dépourvus d'opérandes valides à gauche ou à droite. La tentative de résolution d'une opérande manquante lève une exception sémantique contrôlée au lieu d'émettre des instructions LLVM corrompues.

## 3. Compilation Multi-Fichiers et Concurrence (`OrchestratorInclude`)

**Composant :** `OrchestratorInclude`
**Rôle :** Coordonner la compilation parallèle des fichiers via `llvm::ThreadPool`.

### Cas limite 1 : Dépendances circulaires d'inclusion

* **Scénario :**
  ```
  fichier_a.p inclut fichier_b.p
  fichier_b.p inclut fichier_c.p
  fichier_c.p inclut fichier_a.p  ← cycle potentiel
  ```
* **Traitement :**
  1. À chaque directive `include`, le chemin absolu du fichier cible est canonisé via `std::filesystem::absolute`.
  2. L'accès à l'ensemble `_alreadyIncludedFiles` est verrouillé par un mutex (`std::lock_guard<std::mutex>`).
  3. Si le chemin est déjà enregistré dans le set, la demande d'inclusion est ignorée immédiatement.
  4. Si le chemin est nouveau, il est inséré dans l'ensemble et une nouvelle unité `UnitCompilation` est ajoutée au pool de threads.
* **Résultat :** La boucle infinie d'inclusion et la prolifération de threads sont strictement neutralisées en $O(\log F)$ (où $F$ est le nombre de fichiers du projet).

### Cas limite 2 : Exception levée au sein d'un thread de compilation

* **Scénario :** Une erreur de syntaxe ou d'analyse survient dans un fichier traité au sein d'un thread d'arrière-plan pendant que les autres fichiers continuent leur Passe 1.
* **Traitement :**
  1. Chaque tâche asynchrone encadre l'exécution de l'unité (`unit->pass1()` ou `unit->pass2()`) dans un bloc `try / catch (const CompilationError& e)`.
  2. À l'interception, le thread acquiert le mutex global, consigne le message d'erreur avec le nom du fichier concerné et active le drapeau atomique `_hasErrors = true`.
  3. L'orchestrateur effectue `_threads.wait()` pour garantir que toutes les tâches de la Passe 1 s'achèvent proprement.
  4. Si `_hasErrors` est vérifié à l'issue de la Passe 1, l'orchestrateur annule intégralement le déclenchement de la Passe 2 (génération de code LLVM).
* **Résultat :** Aucune instruction LLVM n'est émise à partir d'un état sémantique incomplet ou invalide. Les ressources allouées en mémoire arène sont libérées sans fuite.

## 4. Alignement Mémoire et Sûreté de Typage

**Composants :** `ContextGenCode`, génération LLVM
**Rôle :** Prévention des corruptions mémoire silencieuses lors de l'émission d'IR.

### Cas limite : Assignation entre types scalaires de largeurs incompatibles

* **Scénario :** Assignation d'un scalaire 64 bits dans un registre 32 bits, ou conversion flottant / entier sans tronquage explicite.
* **Traitement :** La génération de code impose un mécanisme de conversion explicite (`creerAutoCast`). Les opérations de conversion utilisent les instructions adaptées du modèle LLVM SSA (`CreateFPTrunc`, `CreateFPExt`, `CreateSExt`, `CreateTrunc`).
* **Vérification mémoire :** La compilation en mode débogage (`python3 debug.py`) active en continu **AddressSanitizer (ASAN)** (`-fsanitize=address`) et **UndefinedBehaviorSanitizer (UBSan)** (`-fsanitize=undefined`), détectant les erreurs prises en charge sur les chemins instrumentés effectivement exécutés. Ces outils ne prouvent pas l’absence de défauts et n’instrumentent pas automatiquement les programmes produits par Prysma.

## 5. Synthèse des Protections

| Composant | Risque / Cas Limite | Mécanisme de Protection | Comportement Résultant |
| :--- | :--- | :--- | :--- |
| **Lexer** | Fichier source vide | Insertion garantie de `TOKEN_EOF` | Unité minimale valide sans crash |
| **Lexer** | Chaîne non fermée en fin de fichier | Contrôle de borne `cursor < end` | Arrêt immédiat avec numéro de ligne |
| **Lexer** | Données binaires corrompues | Table LUT 256 octets (`CC_OTHER`) | Rejet propre sans dépassement mémoire |
| **Parser** | Parenthèses mal formées | Validation structurelle préalable | Exception syntaxique levée sans allocation |
| **Orchestrator** | Dépendances d'inclusions cycliques | Déduplication par ensemble protégé par mutex | Résolution sans récursion infinie |
| **Orchestrator** | Échec de parsing d'un module | Détection atomique des erreurs + barrière | Annulation de la Passe 2 (fail-fast) |
| **Backend** | Incompatibilité de représentation | Cast explicite LLVM SSA + ASAN/UBSan | Absence de comportement indéfini |
