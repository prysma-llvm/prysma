# Feuille de Route Technique - Compilateur Prysma

Ce document synthétise l'état des capacités techniques implémentées dans le compilateur Prysma ainsi que les objectifs d'ingénierie planifiés pour les versions futures.

## 1. Capacités Techniques Actuelles (Stables)

### Cœur du Langage et Syntaxe
* **Types Primitifs :** Entiers signés (`int32`, `int64`), flottants IEEE 754 (`float`), booléens (`bool`), caractères (`char`), chaînes textuelles (`string`), pointeurs opaques (`ptr`), type unitaire (`void`).
* **Opérations Arithmétiques et Logiques :** Opérateurs scalaires (`+`, `-`, `*`, `/`, `%`), comparaisons (`==`, `!=`, `<`, `>`, `<=`, `>=`), et connecteurs booléens (`&&`, `||`, `!`).
* **Structures de Contrôle :**
  * Instructions conditionnelles `if-else`.
  * Boucles itératives `while` et `for`.
  * Instruction neutre `pass;`.
* **Fonctions :** Fonctions récursives, passage par valeur, passage par référence (`ref`), déréférencement (`unref`), retour de valeurs scalaires et stockage direct du résultat d'un appel dans une variable (`dec <type> v = call <nom>(...);`).
* **Gestion des Identifiants :** Déclaration explicite (`dec`), affectation (`aff`), portées lexicales locales avec pile de contextes.

### Modèle Objet
* **Classes :** Définition de structures de données encapsulant attributs et méthodes.
* **Sections d'Accès :** Modificateurs de visibilité `public:`, `private:` et `protected:`.
* **Héritage :** Dérivation simple entre classes (`class Enfant : Parent`).
* **Polymorphisme :** Table virtuelle dynamique (VTable) pour la résolution des méthodes au runtime.

### Gestion de la Mémoire
* **Allocation sur le Tas (Heap) :** Opérateurs système `new` et `delete` pour la gestion explicite du cycle de vie des objets.
* **Allocation Interne (Arène) :** Utilisation d'un allocateur par arène (`llvm::BumpPtrAllocator`) pour les nœuds de l'AST, assurant la contiguïté mémoire et supprimant les fuites sans parcours de désallocation.

### Infrastructure et Outillage
* **Compilation Multi-fichiers :** Directive native `include "fichier.p";` avec déduplication et prévention des inclusions cycliques.
* **Parallélisme :** Pipeline en 2 passes exécuté via `llvm::ThreadPool`.
* **Environnement Centralisé Docker :** Toolchain complète (LLVM 18, Clang 18, LLD, CMake, ccache, debugpy, LLDB) encapsulée dans une image conteneurisée reproductible.
* **Intégration VS Code :** Profil préconfiguré, coloration syntaxique des fichiers `.p`, compilation à la volée via `Ctrl+Shift+B` et indexation Clangd sans dépendance locale.
* **Visualisation et Débogage :** Export graphique optionnel de l'AST via Graphviz (`--graphviz`) et injection continue des sanitizers (ASAN / UBSan) en mode debug.

## 2. Objectifs d'Ingénierie Planifiés

### Court Terme (v1.1)
* **Parseur d'Expressions de Pratt :** Remplacement de la chaîne de responsabilité par un parseur de Pratt pour ramener la complexité d'évaluation des équations de $O(k \cdot e^2)$ à $O(e)$.
* **Interopérabilité C (`extern`) :** Déclaration de fonctions externes C sans adaptateur manuel pour lier directement les bibliothèques système libc et libm.
* **Structures de Données Standards (StdLib) :** Développement d'un module de vecteurs dynamiques et de tableaux redimensionnables.

### Moyen Terme (v1.2)
* **Pool d'Internement de Chaînes (String Interning) :** Centralisation des identifiants dans une table de chaînes uniques pour remplacer les comparaisons lexicographiques par des comparaisons d'adresses mémoire.
* **Dispatch Statique des Nœuds AST :** Transition du parcours virtuel `accept` / `visit` vers un dispatch statique par identifiant contigu, optimisant la prédiction de branchement du processeur lors de l'émission LLVM.
* **Espaces de Noms (Namespaces) :** Isolation explicite des symboles par module pour éviter les collisions dans les projets de grande taille.

### Long Terme (Vision)
* **Compilation JIT (Just-In-Time) :** Mode d'exécution immédiat en mémoire pour l'outillage de test et le scripting.
* **Passes d'Optimisation SSA Dédiées :** Écriture de passes personnalisées sur le graphe LLVM IR pour exploiter les propriétés sémantiques propres à Prysma.
* **Assembleur Intégré (`asm`) :** Bloc d'instructions machine directes pour les routines critiques dépendantes du matériel.
