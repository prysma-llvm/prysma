# Écrire un test d’intégration Prysma

[English version](../en/integration_tests.md)

Un test d’intégration compile et exécute un programme Prysma. On place des checkpoints après les opérations qu’on veut vérifier, par exemple une affectation, un retour de fonction ou un tour de boucle. Chaque checkpoint peut vérifier une valeur, un passage dans une branche, ou les deux.

## Où mettre le test

Pour un scénario fixe, créez un fichier `.p.jinja` dans [tests/unit_tests/templates/scenarios](../../tests/unit_tests/templates/scenarios). Le point d’entrée est `fn int32 main()`. Si le test utilise des `include`, gardez les fichiers du projet ensemble et utilisez des chemins relatifs vers les futurs `.p`.

Les templates paramétrés se trouvent dans [tests/unit_tests/templates](../../tests/unit_tests/templates), en dehors de `scenarios`. Le générateur les rend avec les types et les données du scénario. Le [test de mémoire dynamique](../../tests/unit_tests/templates/test_allocation_dynamic_memory/mem_extreme.p.jinja) en est un exemple.

## Vérifier une valeur et une branche

```jinja2
fn int32 main() {
    dec int32 a = 1;
    dec int32 b = 1;

    {{ assert_eq(1, "a", trace_positions=1) }}
    {{ assert_eq(1, "b", trace_positions=2) }}

    if (a == 1 && b == 1) {
        {{ checkpoint(trace_positions=3) }}
    } else {
        {{ checkpoint(trace_positions=[]) }}
    }

    return 0;
}
```

`assert_eq(attendu, "expression", trace_positions=...)` vérifie le passage et la valeur de l’expression. Pour vérifier plusieurs valeurs au même passage, écrivez par exemple `assert_eq([1, 1], ["a", "b"], trace_positions=1)`. `checkpoint(trace_positions=...)` vérifie uniquement le passage.

**`trace_positions` est obligatoire.** Il donne la position attendue du passage pendant l’exécution du programme Prysma. La trace commence à `1` et ses positions se suivent sur toute l’exécution, fichiers inclus compris.

- `trace_positions=3` attend un seul passage, au troisième événement de la trace.
- `trace_positions=[2, 4]` attend deux passages, aux événements 2 et 4. Avec `assert_eq`, donnez une valeur attendue par passage, dans le même ordre.
- `checkpoint(trace_positions=[])` attend zéro passage. Si le programme atteint ce point, le test échoue. Ce checkpoint ne réserve aucune position dans la trace attendue.

Le générateur attribue aussi un identifiant à chaque checkpoint dans l’ordre du rendu de chaque fichier `.p`. Cet identifiant et la ligne du fichier servent à localiser le checkpoint dans les diagnostics. Le checkpoint qui vérifie `outcome` illustre cette distinction. Il porte l’identifiant 4 et utilise `trace_positions=3`, car la branche `else` doit rester inactive. [Voir le template](../../tests/unit_tests/templates/scenarios/project_test_edge_case/test_functional_variable_bool/test_logical_not_false.p.jinja).

### Vérifier une boucle

Dans le [test de boucle simple](../../tests/unit_tests/templates/scenarios/project_test_edge_case/test_loop/test_loop_simple.p.jinja), le checkpoint `2` est placé après l’incrémentation. Il doit afficher `1`, puis `2`, jusqu’à `5` aux événements `2` à `6`.

```jinja2
{{ assert_eq([1, 2, 3, 4, 5], "i", trace_positions=[2, 3, 4, 5, 6]) }}
```

### Attendre une erreur de compilation

Placez `// @compile-error "expression régulière"` sur la ligne à tester. Le lanceur vérifie que le compilateur rejette le programme avec un diagnostic qui correspond à l’expression. Ce scénario s’arrête à la compilation. Voir le [test d’écriture hors limites](../../tests/unit_tests/templates/array_test_out_of_bounds.p.jinja).

## Comment ça fonctionne

1. [generate_tests.py](../../tests/unit_tests/generate_tests.py) rend les templates dans `tests/unit_tests/build/prysma_code_generation`. Les helpers génèrent les `call print(...)` et les commentaires `// @expect`.
2. [orchestrator_test.py](../../tests/unit_tests/orchestrator_test.py) repère les points d’entrée. [checkpoints.py](../../tests/unit_tests/checkpoints.py) lit les attentes dans les `.p` générés et retrouve le fichier et la ligne de chaque checkpoint.
3. [unit_test.py](../../tests/unit_tests/unit_test.py) compile chaque projet dans un dossier temporaire et lance son exécutable. Le lanceur compare la trace attendue à la sortie du programme et signale la première divergence avec `File`, `Checkpoint`, `Line`, `Expected`, `Actual` et `Error`. La ligne affichée est celle du `.p` généré.

## Lancer les tests

Depuis la racine du dépôt, lancez la compilation du compilateur, les tests C++ et les tests d’intégration Prysma avec cette commande.

```bash
python3 tests/run_unit_tests.py --existing
```

Si `build/Prysma` est déjà compilé, vous pouvez seulement régénérer les programmes et les tester. Ces commandes sont aussi utilisables dans l’[environnement Docker du projet](installation/docker_compilation.md).

```bash
python3 tests/unit_tests/generate_tests.py
python3 tests/unit_tests/orchestrator_test.py
```
