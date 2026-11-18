# Writing a Prysma integration test

[Version française](../fr/integration_tests.md)

An integration test compiles and runs a Prysma program. Place checkpoints after the operations you want to check, such as an assignment, a function return, or a loop iteration. A checkpoint can check a value, whether a branch was taken, or both.

## Where to put the test

For a fixed scenario, create a `.p.jinja` file in [tests/unit_tests/templates/scenarios](../../tests/unit_tests/templates/scenarios). The entry point is `fn int32 main()`. If the test uses `include`, keep the project files together and use relative paths to the `.p` files that will be generated.

Parameterized templates live in [tests/unit_tests/templates](../../tests/unit_tests/templates), outside `scenarios`. The generator renders them with the types and data for each case. See the [dynamic memory test](../../tests/unit_tests/templates/test_allocation_dynamic_memory/mem_extreme.p.jinja) for an example.

## Checking a value and a branch

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

`assert_eq(expected, "expression", trace_positions=...)` checks that execution reaches that point and verifies the expression's value. To check several values in one visit, use `assert_eq([1, 1], ["a", "b"], trace_positions=1)`. `checkpoint(trace_positions=...)` checks only that execution reaches the point.

**`trace_positions` is required.** It gives the expected position of a visit during execution of the Prysma program. The trace starts at `1` and its positions run consecutively throughout execution, including visits in files brought in with `include`.

- `trace_positions=3` expects one visit, as the third event in the trace.
- `trace_positions=[2, 4]` expects two visits, at events 2 and 4. With `assert_eq`, supply one expected value per visit in the same order.
- `checkpoint(trace_positions=[])` expects zero visits. If execution reaches that point, the test fails. This checkpoint takes up no position in the expected trace.

The generator also assigns each checkpoint an ID in render order within each `.p` file. Diagnostics use the ID and source line to locate the checkpoint. For example, the checkpoint that checks `outcome` has ID 4 and uses `trace_positions=3` because the `else` branch should never run. [See the template](../../tests/unit_tests/templates/scenarios/project_test_edge_case/test_functional_variable_bool/test_logical_not_false.p.jinja).

### Checking a loop

In the [simple loop test](../../tests/unit_tests/templates/scenarios/project_test_edge_case/test_loop/test_loop_simple.p.jinja), checkpoint `2` comes after the increment. It should print the values `1` through `5` at events `2` through `6`.

```jinja2
{{ assert_eq([1, 2, 3, 4, 5], "i", trace_positions=[2, 3, 4, 5, 6]) }}
```

### Expecting a compilation error

Put `// @compile-error "regular expression"` on the line you want to test. The runner checks that the compiler rejects the program with a diagnostic matching the expression. This scenario stops at compilation. See the [out-of-bounds write test](../../tests/unit_tests/templates/array_test_out_of_bounds.p.jinja).

## How it works

1. [generate_tests.py](../../tests/unit_tests/generate_tests.py) renders the templates into `tests/unit_tests/build/prysma_code_generation`. The helpers generate the `call print(...)` statements and `// @expect` comments.
2. [orchestrator_test.py](../../tests/unit_tests/orchestrator_test.py) finds the entry points. [checkpoints.py](../../tests/unit_tests/checkpoints.py) reads the expectations from the generated `.p` files and records the file and line for each checkpoint.
3. [unit_test.py](../../tests/unit_tests/unit_test.py) compiles each project in a temporary folder and runs its executable. The runner compares the output with the expected trace and reports the first difference using `File`, `Checkpoint`, `Line`, `Expected`, `Actual`, and `Error`. The line number refers to the generated `.p`.

## Running the tests

From the repository root, run this command to build the compiler and run the C++ and Prysma integration tests.

```bash
python3 tests/run_unit_tests.py --existing
```

If `build/Prysma` is already built, you can just regenerate the programs and run them. These commands also work in the [project’s Docker environment](installation/docker_compilation.md).

```bash
python3 tests/unit_tests/generate_tests.py
python3 tests/unit_tests/orchestrator_test.py
```
