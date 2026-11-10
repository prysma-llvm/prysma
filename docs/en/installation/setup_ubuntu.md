# Native Ubuntu Installation (Non-Container Alternative)

> **Important:** This guide documents manual direct installation on an Ubuntu host. The official and supported method for compiling Prysma relies on the centralized Docker architecture: refer to the [Docker Compilation Guide](docker_compilation.md) and the [VS Code Environment Guide](vscode_environment.md).

## 1. System Prerequisites

Install the LLVM 18 toolchain, Clang compiler, LLD linker, and build dependencies:

```bash
sudo apt update
sudo apt install -y llvm-18 llvm-18-dev clang-18 lld-18 bear graphviz cmake ninja-build ccache python3 python3-venv
```

## 2. Python Virtual Environment

Set up a virtual environment to execute the compiler's internal AST and parser generators:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## 3. Project Compilation

To compile the project in debug mode with AddressSanitizer:

```bash
python3 debug.py
```

To compile the project in release mode:

```bash
python3 build.py
```

## 4. VS Code Configuration (`tasks.json`)

When working natively without Docker, configure the on-the-fly compilation task (`Ctrl+Shift+B`) to run the local binary in `build/`:

```json
{
  "label": "Compile Prysma (Native)",
  "type": "shell",
  "command": "./build/Prysma",
  "args": ["${file}"],
  "options": {
    "cwd": "${workspaceFolder}"
  },
  "group": {
    "kind": "build",
    "isDefault": true
  },
  "presentation": {
    "reveal": "silent",
    "panel": "shared",
    "clear": true,
    "focus": false
  },
  "problemMatcher": {
    "owner": "prysma_language",
    "fileLocation": ["absolute"],
    "pattern": {
      "regexp": "^([^:]+):(\\d+):(\\d+):\\s+(?:Error|error|warning):\\s+(.*)$",
      "file": 1,
      "line": 2,
      "column": 3,
      "message": 4
    }
  }
}
```

## 5. Prysma Syntax Highlighting

To package and install the local syntax highlighting extension from source:

```bash
cd .vscode/SyntaxeCouleurPrysma
npm install
npx vsce package
```

Then install the generated `.vsix` file in VS Code (*Extensions view > `...` menu > Install from VSIX...*).
