# Analyse Technique : Projet Prysma

Ce document présente l'évaluation technique des choix d'ingénierie retenus pour le compilateur Prysma, notamment le choix de l'infrastructure de génération de code et l'organisation du backend.

## 1. Objectifs Techniques

L'objectif de Prysma est de proposer un langage système avec un modèle d'exécution déterministe et une gestion mémoire sans ramasse-miettes. Trois approches de génération de code ont été évaluées :

* **LLVM IR (Approche retenue) :** Représentation intermédiaire en forme SSA (Single Static Assignment). Offre l'accès direct aux passes d'optimisation avancées de LLVM (vectorisation, élimination de code mort, inline) et supporte nativement la compilation croisée vers x86-64, ARM64 et WebAssembly.
* **Backend x86-64 manuel :** Offre un contrôle absolu sur le code machine généré, mais impose la gestion manuelle de l'allocation de registres (coloration de graphes, linear scan), de l'alignement de la pile et du respect des conventions d'appel de l'ABI System V AMD64. Le coût de maintenance est disproportionné face au gain de contrôle.
* **Transpilation vers le langage C :** Simplifie la mise en œuvre initiale, mais délègue le contrôle du modèle mémoire et limite la capacité à émettre des représentations bas niveau exactes nécessaires au débogage précis via DWARF.

### Exemple de LLVM IR cible (LLVM 18)

LLVM 18 utilise le modèle de pointeurs opaques (`ptr`), éliminant la distinction entre pointeurs typés au niveau de l'IR :

```llvm
%1 = load i32, ptr %ptr_x         ; charge l'entier 32 bits depuis l'adresse mémoire %ptr_x
%resultat = add i32 %1, 5         ; additionne 5 à la valeur chargée
```

## 2. Défis de la Génération de Code Machine

### Problématiques d'un Backend Assembleur Dédié

1. **Allocation des registres processeur :** L'architecture x86-64 dispose de 16 registres généraux. L'allocation optimale sans conflit nécessite des algorithmes complexes (débordement sur la pile / *spilling*, analyse de durée de vie des variables).
2. **Conventions d'appel (ABI) :** Implémentation manuelle du passage des arguments (registres `rdi`, `rsi`, `rdx`, etc.), de l'alignement de la pile sur 16 octets et de la gestion des frames d'activation.
3. **Optimisations dépendantes de la cible :** La réorganisation d'instructions pour minimiser les latences de pipeline ou l'exploitation des jeux d'instructions vectoriels (AVX, NEON) exigerait une réécriture spécifique par micro-architecture.

### Bénéfices de l'Infrastructure LLVM

* **Portabilité :** Le frontend Prysma émet un flux IR indépendant de la machine hôte ; le backend LLVM se charge d'adapter le binaire à l'architecture cible.
* **Optimisations prouvées :** Intégration immédiate des algorithmes de simplification d'instructions, de propagation de constantes et de vectorisation automatique.
* **Écosystème et Outillage :** Compatibilité native avec l'éditeur de liens LLD, les débogueurs GDB/LLDB et les analyseurs de performance matérielle.

## 3. Références Techniques

* **Rust :** Langage système compilé via LLVM, référence en matière de gestion mémoire déterministe et d'optimisations SSA.
* **Kaleidoscope (Tutoriel LLVM) :** Référence didactique fournie par l'équipe LLVM détaillant la transition d'un AST vers les constructeurs `IRBuilder`.
* **Zig :** Langage de programmation système mettant l'accent sur l'absence de comportement caché, la gestion explicite de la mémoire et la simplicité de l'outillage.
