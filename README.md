# Prysma Programming Language
<div align="center">

  <img src="img/prysma.svg" alt="Prysma Logo" width="250"/>
 
<br>

[![CI](https://github.com/Zyphorah/Prysma/actions/workflows/ci.yml/badge.svg)](https://github.com/Zyphorah/Prysma/actions/workflows/ci.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![LLVM](https://img.shields.io/badge/LLVM-18-red.svg)](https://llvm.org/)
</div>

Prysma is a high-performance system programming language, compiled to the **LLVM 18** infrastructure. The project prioritizes deterministic resource control and a robust modular architecture.

## Technical Capabilities

  * **Backend:** Intermediate Representation (IR) generation via LLVM 18 in SSA (Single Static Assignment) form.
  * **Object Model:** Native implementation of classes, inheritance, and dynamic polymorphism via VTable.
  * **Memory Management:** Manual heap control via the `new` and `delete` operators. Internal allocation optimized by **Bump Pointer Allocator** (Arena).
  * **Parallelism:** Multi-file compilation orchestration via `llvm::ThreadPool`.
  * **Static Analysis:** Strong typing system with secure auto-casting and circular inclusion dependency detection.

## Compiler Architecture

The processing pipeline follows a strict linear structure:

1.  **Frontend:** Lexical analysis (Lexer) and recursive descent parsing.
2.  **Intermediate Representation:** Construction of an Abstract Syntax Tree (AST) based on the **Composite** design pattern.
3.  **Code Generation:** Translation of the AST into LLVM IR via the **Visitor** design pattern.
4.  **Backend:** Optimizations and native binary generation by the LLVM infrastructure.

## Syntax Overview

```rust
fn int32 main() {
    dec string message = "Prysma system operational";
    call print(ref message);
    return 0;
}
```

## Object-oriented programming

> Currently in experimental phase. This feature set is incomplete and may exhibit instabilities.

## Official Compilation Environment (Docker)

Prysma's official build system is designed around a **centralized compilation tool architecture using Docker**.

This design encapsulates the entire compiler toolchain (LLVM 18, Clang 18, LLD, CMake, ccache, analysis tools, and Python venv) into a unified, reproducible container. It guarantees identical builds across environments and eliminates the need to install heavy dependencies directly on the host machine.

### Docker Installation

Ensure Docker is installed and running on your host system:

| Host OS | Installation Guide |
| :--- | :--- |
| **Arch Linux** | [View Arch Linux Guide](docs/en/installation/docker/arch_linux.md) |
| **Debian** | [View Debian Guide](docs/en/installation/docker/debian.md) |
| **Ubuntu** | [View Ubuntu Guide](docs/en/installation/docker/ubuntu.md) |
| **Windows (PowerShell)** | [View Windows Guide](docs/en/installation/docker/windows_powershell.md) |

### Quick Start

This environment serves as the official workspace for the project. It is designed both for users who simply want to compile and test Prysma, and for those looking to actively contribute to its development.

A baseline configuration is provided to streamline development in **VS Code** (standard / "blue" VS Code) via the `.vscode` directory and the `docker-cli.sh` script. It notably enables compiling the currently active Prysma source file directly with `Ctrl+Shift+B` (used exclusively to compile files with the `.p` extension), alongside debugging support and seamless Clangd code indexing without requiring any local toolchain on the host.

#### 1. Build the compilation image:

```bash
docker build --target debug -t prysma-debug -f docker/compiler/Dockerfile .
```

#### 2. Open an interactive container session:

```bash
docker run --rm -it -v "$(pwd)":/workspace prysma-debug
```

#### 3. Compilation orchestration (`build.py` & `debug.py`):

Inside the container (in `/workspace`), compilation is orchestrated by two Python scripts acting as a unified interface:

* **Why a Python wrapper?** Before calling CMake and Clang, the script automatically triggers internal code generators (generating the AST, visitor interfaces, GraphViz exporter, and parser), enables `ccache`, and dynamically injects compiler flags without requiring complex manual commands.
* **`python3 build.py` (Release mode)**: Generates code and builds an aggressively optimized release binary (`-O3`, `-march=native`, `-flto`, and the `lld` linker).
* **`python3 debug.py` (Debug mode)**: Generates code and injects extensive debugging and instrumentation flags (`-g3`, `-O0`, stack protector `-fstack-protector-all`, assertions, alongside memory detectors AddressSanitizer `-fsanitize=address` and UndefinedBehaviorSanitizer `-fsanitize=undefined`).

*(You can also compile directly from the host terminal without opening an interactive shell, for instance: `docker run --rm -v "$(pwd)":/workspace prysma-debug python3 build.py`).*

### Development Environment and Debugging in VS Code

The repository provides out-of-the-box integration for **VS Code** (standard / "blue" VS Code), combining the usability of your host IDE with the containerized build toolchain:

* **Preconfigured Profile (`.vscode/Prysma.code-profile`)**: Installs recommended extensions (Clangd, LLDB DAP, Debugpy) and fine-tunes settings in a single click.
* **Official Syntax Highlighting (`.vscode/SyntaxeCouleurPrysma`)**: Internal extension providing language grammar, delimiters, and bracket coloring for `.p` files.
* **On-the-fly Compilation**: Pressing `Ctrl+Shift+B` immediately compiles the active `.p` source file using the Prysma binary inside the container.
* **Integrated Interactive Debugging (C++ & Python)**: Ready-to-run `launch.json` profiles (`F5`) to debug the C++ compiler via LLDB (port 1234), unit tests, and Python generator scripts via debugpy (port 5678).
* **Transparent Clangd Code Intelligence**: Accurate real-time semantic analysis and navigation without requiring local LLVM on your host system.

Refer to the [VS Code Environment Guide](docs/en/installation/vscode_environment.md) for step-by-step instructions on profile import, syntax extension packaging, and debugger workflows.

For advanced compilation options and benchmark execution, refer to the [Docker Architecture & Compilation Guide](docs/en/installation/docker_compilation.md).

## Documentation

All technical documentation and setup guides are maintained in the `/docs` directory:

* **Compilation & Environment:**
  * [Centralized Architecture & Docker Guide](docs/en/installation/docker_compilation.md)
  * [VS Code Development & Debugging Guide](docs/en/installation/vscode_environment.md)
  * Platform Guides: [Arch Linux](docs/en/installation/docker/arch_linux.md) • [Debian](docs/en/installation/docker/debian.md) • [Ubuntu](docs/en/installation/docker/ubuntu.md) • [Windows](docs/en/installation/docker/windows_powershell.md)
  * [Local Native Ubuntu Setup (legacy)](docs/en/installation/setup_ubuntu.md)
* **Architecture & Design:**
  * [Architecture Analysis](docs/en/architecture.md)
  * [Robustness and Security Analysis](docs/en/robustness.md)
  * [Language Specifications](docs/en/prysma.md)
  * [Technological Choices Report](docs/en/analysis/technology_choices.md)

## License

This project is distributed under the **GPL v3** license. Consult the `LICENSE` file for legal details.

## Official Maintainers

* Raphael Arseneault ([@Zyphorah](https://github.com/Zyphorah))
* Félix-Olivier Dumas ([@unrays](https://github.com/unrays))
