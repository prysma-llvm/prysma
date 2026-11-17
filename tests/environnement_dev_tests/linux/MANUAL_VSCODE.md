# VS Code Validation (Linux)

Guide to validate VS Code workflows (build tasks, LLDB, and debugpy) using Docker on native Linux.

## 1. Prysma Compilation (Ctrl+Shift+B)

1. Run the fixture generator in Docker (`python3 tests/unit_tests/generate_tests.py`), then open `tests/unit_tests/build/prysma_code_generation/scenarios/project_test_multi_thread_edge_case_file/test_depth/main.p`.
2. Press `Ctrl+Shift+B` to run the default build task (`Compile Prysma Program`).
3. Check the integrated terminal: `docker-cli.sh` runs the compiler inside the container and produces the compiled output without errors.

## 2. C++ Compiler Debugging (LLDB)

1. Open `src/main.cpp` and set a breakpoint on an executable line in `main()`.
2. Press `F5` with the `C++: Debug Prysma in Docker (LLDB)` launch configuration.
3. The pre-launch task runs `docker-debug-lldb.sh`, which starts `lldb-server-18` on port `1234`.
4. The debugger attaches, halts at the breakpoint, and local variables and call frames appear in the Run & Debug view.
5. Step over (`F10`) and resume (`F5`) to verify session continuity.

## 3. Python Debugging (debugpy)

1. Open `debug.py` and set a breakpoint on an executable line in `main()`.
2. Launch `Python: Debug in Docker`.
3. `docker-debug-python.sh` starts `debugpy` on port `5678`.
4. The debugger attaches, pauses at the breakpoint, and source paths match correctly between the host and `/workspace`.

## 4. Diagnostic Reporting (Problem Matcher)

1. Introduce a syntax error in a `.p` file (e.g. invalid token).
2. Trigger compilation with `Ctrl+Shift+B`.
3. Verify that the syntax error appears in the **Problems** panel (`Ctrl+Shift+M`) with the exact file path, line number, and column.

## 5. Teardown & Ports

1. Stop any active debug sessions (`Shift+F5`).
2. Verify debug containers are terminated and ports 1234 and 5678 are released:
   ```bash
   docker ps
   ```
