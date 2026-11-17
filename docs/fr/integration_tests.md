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

    {{ assert_eq(1, 1, "a") }}
    {{ assert_eq(2, 1, "b") }}

    if (a == 1 && b == 1) {
        {{ checkpoint(3) }}
    } else {
        {{ checkpoint(4, at=[]) }}
    }

    return 0;
}
```

`assert_eq(id, attendu, "expression")` vérifie la valeur affichée. On peut aussi vérifier plusieurs valeurs au même point avec `assert_eq(1, [1, 1], ["a", "b"])`. `checkpoint(id)` vérifie le passage à cet endroit. `at=[]` indique que la branche doit rester inactive.

Les IDs commencent à `1` et suivent l’ordre des checkpoints dans chaque fichier. Par défaut, l’ID indique aussi la position dans la trace. Utilisez `at` quand ces deux ordres diffèrent. Les positions de la trace commencent à `1` et se suivent sur toute l’exécution, fichiers inclus compris.

### Vérifier une boucle

Dans le [test de boucle simple](../../tests/unit_tests/templates/scenarios/project_test_edge_case/test_loop/test_loop_simple.p.jinja), le checkpoint `2` est placé après l’incrémentation. Il doit afficher `1`, puis `2`, jusqu’à `5` aux événements `2` à `6`.

```jinja2
{{ assert_eq(2, [1, 2, 3, 4, 5], "i", at=[2, 3, 4, 5, 6]) }}
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
