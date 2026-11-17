# VS Code Validation (Windows)

Guide to validate VS Code workflows (build tasks, LLDB, and debugpy) using Docker Desktop and PowerShell on Windows.

## 1. Prysma Compilation (Ctrl+Shift+B)

1. Run the fixture generator in Docker (`python3 tests/unit_tests/generate_tests.py`), then open `tests/unit_tests/build/prysma_code_generation/scenarios/project_test_multi_thread_edge_case_file/test_depth/main.p`.
2. Press `Ctrl+Shift+B` to run the `Compile Prysma Program` task.
3. `docker-cli.ps1` normalizes Windows backslashes (`\`) to container forward slashes (`/`) and invokes the compiler inside the container. Compilation succeeds with exit code 0.

## 2. C++ Compiler Debugging (LLDB)

1. Open `src/main.cpp` and set a breakpoint on an executable line in `main()`.
2. Press `F5` with `C++: Debug Prysma in Docker (LLDB)`.
3. The pre-launch task runs `docker-debug-lldb.ps1`, generates `.vscode/lldb_source_map.txt` with normalized paths, and starts `lldb-server-18` on port `1234`.
4. VS Code connects to the server and stops at the breakpoint. Windows source paths resolve correctly to the local file.

## 3. Python Debugging (debugpy)

1. Open `debug.py` and set a breakpoint on an executable line in `main()`.
2. Launch `Python: Debug in Docker`.
3. `docker-debug-python.ps1` starts `debugpy` on port `5678`.
4. VS Code attaches to port 5678, execution stops at the breakpoint, and source paths map properly between Windows and `/workspace`.

## 4. Diagnostic Reporting (Problem Matcher)

1. Introduce a syntax error in a `.p` file.
2. Press `Ctrl+Shift+B`.
3. The compilation error must appear in the **Problems** panel (`Ctrl+Shift+M`) pointing directly to the active Windows document.

## 5. Teardown & Ports

1. Disconnect active debug sessions.
2. Check that workspace-specific debug containers are removed and ports are freed:
   ```powershell
   docker ps
   ```
