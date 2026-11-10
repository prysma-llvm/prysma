# Development Environment and Debugging in VS Code

This document outlines the VS Code development environment for building, debugging, and testing the Prysma project using Docker.

## 1. Environment Overview

The architecture combines VS Code on your host machine with the isolation of the Docker compilation container:

* **No heavy host dependencies**: LLVM 18, Clang 18, LLD, GDB, and LLDB run exclusively inside the `prysma-debug` container.
* **C++ indexing with Clangd**: The `docker-cli.sh` wrapper translates paths in `compile_commands.json` so host Clangd provides autocompletion and code navigation without requiring local LLVM.
* **Full debugging support (C++ and Python)**: Launch profiles allow setting breakpoints and inspecting executions inside the container via LLDB and debugpy.
* **Prysma syntax highlighting**: An internal extension handles syntax coloring and delimiter matching for `.p` source files.

## 2. Installing the Default Profile (`.vscode/Prysma.code-profile`)

The `.vscode/Prysma.code-profile` file bundles recommended extensions, formatting rules, and debugger settings.

### Import Steps in VS Code

1. Open VS Code in the root of the `prysma` project.
2. Open the Command Palette (`Ctrl+Shift+P` / `Cmd+Shift+P`), type `Profiles: Import Profile`, and press Enter (or access it through *Settings gear > Profiles > Import Profile...*).
3. Click **Select File...** and choose:  
   `.vscode/Prysma.code-profile`
4. Select the components to import (Settings, Extensions, Keybindings) and click **Create Profile** (or **Import**).

Once applied, VS Code automatically installs recommended extensions (`llvm-vs-code-extensions.vscode-clangd`, `llvm-vs-code-extensions.lldb-dap`, `ms-python.debugpy`, etc.) and configures Clangd as the primary C++ engine.

## 3. Installing Prysma Syntax Highlighting (`SyntaxeCouleurPrysma`)

The `.vscode/SyntaxeCouleurPrysma` directory contains the source code for the extension providing syntax coloring, delimiter pairs, and TextMate grammar for `.p` files.

### Step 1: Package the Extension into a `.vsix` Archive

From your host terminal:

```bash
cd .vscode/SyntaxeCouleurPrysma
npm install
npx vsce package
```

This command generates `prysma-language-support-0.0.2.vsix`.

> **Note:** If Node.js / npm is not installed on your host system, you can run this command in any environment with Node 18+.

### Step 2: Install the `.vsix` Package in VS Code

1. In VS Code's file explorer, navigate to `.vscode/SyntaxeCouleurPrysma`.
2. Right-click the generated `.vsix` file (`prysma-language-support-*.vsix`).
3. Click **Install Extension VSIX** in the context menu (or open Extensions view `Ctrl+Shift+X` > `...` menu > **Install from VSIX...**).
4. Reload the window if prompted (`Ctrl+Shift+P` > `Developer: Reload Window`).

Files ending in `.p` will now have syntax highlighting enabled.

## 4. Integrated Build Tasks (`tasks.json`)

The `.vscode/tasks.json` file provides automated tasks routed through `docker-cli.sh`:

* **Compile Active Prysma File (`Ctrl+Shift+B`)**:  
  The default `Compile Prysma Program` task compiles the currently active `.p` source file by invoking `./build/Prysma` inside the container. Compilation errors and warnings are parsed via `problemMatcher` regex and underlined directly in the editor.
* **Rebuild Prysma (`Build`)**:  
  Executes `debug.py` inside the Docker container to regenerate code and rebuild the compiler with debug symbols and sanitizers.
* **Build Unit Tests (`Build Unit Tests`)**:  
  Compiles the C++ unit test suite under `tests/unit_tests/build`.

## 5. Debugging Configurations (`launch.json`)

The Run and Debug panel (`Ctrl+Shift+D` or `F5`) provides three primary debug profiles:

### C++ Compiler Debugging (`C++: Debug Prysma in Docker (LLDB)`)
Allows debugging the Prysma compiler executable with LLDB.  
It automatically starts `lldb-server-18` in the container listening on port `1234` via the `Start LLDB Server (Docker)` pre-launch task. VS Code attaches to the remote server (`gdb-remote 127.0.0.1:1234`) with source mapping from `/workspace` to your local `${workspaceFolder}`. You can set breakpoints and inspect variables directly in VS Code.

### Unit Tests Debugging (`C++: Debug Unit Tests in Docker (LLDB)`)
Automatically builds the unit tests via `Build Unit Tests`, launches `lldb-server` on `PrysmaTests`, and attaches LLDB to step through test failures.

### Python Debugging (`Python: Debug in Docker`)
Allows debugging AST generators, the parser, or build orchestration scripts (`debug.py` / `build.py`).  
The `Start Python Debug Server (Docker)` task starts `debugpy` inside the container listening on port `5678` via `docker/docker-debug-python.sh`. VS Code connects with path mappings between the local workspace and `/workspace`. The container terminates automatically when debugging ends.

## 6. C++ Code Navigation & Autocompletion (Clangd)

The `.vscode/settings.json` file configures **Clangd** as the language engine:
* `docker-cli.sh` translates `/workspace` paths to absolute host paths in `build/compile_commands.json` after every build.
* Internal LLVM headers are bridged via `.clangd-includes`.
* Clangd provides semantic analysis, "Go to Definition" (`F12`), real-time Clang-Tidy diagnostics, and inlay hints.
