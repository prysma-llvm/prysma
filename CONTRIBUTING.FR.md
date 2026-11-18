# Guide de Contribution : Compilateur Prysma

Le développement de Prysma repose sur une rigueur architecturale stricte. L'objectif principal est de maintenir un compilateur déterministe et performant. Toute contribution doit s'aligner sur ces standards d'ingénierie.

Ce document définit les protocoles pour soumettre des modifications.

## 1. Signalement d'Anomalies (Issues)

Avant de soumettre un rapport, vérifiez que l'anomalie n'a pas déjà été documentée. Tout signalement doit être factuel et reproductible.

Votre rapport doit obligatoirement inclure :
* **Le code source Prysma** minimal permettant de reproduire l'erreur (extension `.p`).
* **L'environnement d'exécution** (version de LLVM, distribution ou conteneur).
* **Les traces de diagnostic :** Si le crash survient à la compilation, fournissez la trace LLVM IR. Si c'est une erreur mémoire, fournissez la sortie d'AddressSanitizer (ASAN) ou de GDB/LLDB.

## 2. Standards de Développement (C++)

Le cœur du compilateur est écrit en C++23. Les règles suivantes sont obligatoires :

* **Séparation des préoccupations :** Respectez le découplage strict entre le Frontend (Lexer/Parser) et le Backend (LLVM).
* **Métaprogrammation :** Ne créez pas de nœuds AST manuellement. Toute nouvelle instruction ou expression doit être ajoutée via le système de génération Python/Jinja2 (répertoire `/generation`).
* **Gestion de la mémoire :** L'allocation interne des nœuds AST est gérée par l'arène contiguë (`llvm::BumpPtrAllocator`). L'utilisation de pointeurs intelligents standards (`std::shared_ptr`) est prohibée dans le flux critique de l'AST pour préserver la localité du cache. Aucune fuite mémoire n'est tolérée.
* **Patrons de conception :** Les modifications de la génération de code doivent respecter le contrat du patron **Visitor**.

## 3. Protocole de Soumission (Pull Requests)

1. Créez une branche dérivée (*fork*) et travaillez sur une branche thématique (`feat/nom-fonctionnalite` ou `fix/nom-bug`).
2. Rédigez des messages de commit explicites et structurés en anglais (ex. : `feat: Implement string concatenation in LLVM backend` ou `fix: Resolve memory leak in VTable generation`).
3. Le code doit compiler sans aucun avertissement (*warning*).
4. **Validation obligatoire :** Vous devez exécuter l'intégralité de la suite de tests (unitaires et d'intégration) avant toute soumission :
   ```bash
   python3 debug.py
   ```

## 4. Tests d’intégration Prysma

Écrivez les scénarios dans `tests/unit_tests/templates/scenarios` avec l’extension `.p.jinja`. Les autres templates de `tests/unit_tests/templates` servent aux variantes paramétrées par type.

```jinja2
{{ assert_eq(15, "value", trace_positions=1) }}
{{ checkpoint(trace_positions=2) }}
{{ checkpoint(trace_positions=[]) }}
{{ assert_eq([10, 20], "n", trace_positions=[3, 4]) }}
```

Chaque appel exige `trace_positions` pour indiquer sa position attendue dans la trace d’exécution. La trace commence à `1` et continue à travers les fichiers inclus. Un entier indique un seul passage, une liste indique des passages répétés et `checkpoint(trace_positions=[])` exige que ce point ne soit jamais atteint. Le générateur attribue séparément les identifiants de checkpoint utilisés dans les diagnostics. Voir le [guide des tests d’intégration](docs/fr/integration_tests.md).

Lancez `python3 tests/run_unit_tests.py --existing` pour générer les `.p` et exécuter la suite. Les fichiers générés se trouvent dans `tests/unit_tests/build/prysma_code_generation`.
