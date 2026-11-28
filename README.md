# Prysma Programming Language
<div align="center">

  <img src="img/prysma.svg" alt="Prysma Logo" width="250"/>
 
<br>

[![CI](https://github.com/Zyphorah/Prysma/actions/workflows/ci.yml/badge.svg)](https://github.com/Zyphorah/Prysma/actions/workflows/ci.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![C++](https://img.shields.io/badge/C++-23-blue.svg)](https://isocpp.org/)
[![LLVM](https://img.shields.io/badge/LLVM-18-red.svg)](https://llvm.org/)
</div>

Prysma is a system programming language compiled to the **LLVM 18** infrastructure. The project focuses on deterministic resource control without a garbage collector, branchless compile-time lookup tables for tokenization, contiguous arena allocation, and parallel multi-file compilation.

[Official documentation](https://prysma-llvm.github.io/prysma/#en/README.md)

## Technical Capabilities

* **LLVM Backend:** Intermediate Representation (IR) generation targeting LLVM 18 SSA form using opaque pointers.
* **Lexical Analysis:** Tokenization powered by compile-time lookup tables (256-byte CharClass LUT) and Little-Endian 64-bit integer comparisons, eliminating dynamic hashing overhead.
* **Memory Management:** Manual heap allocation via `new` and `delete` operators. AST node allocations backed by contiguous arenas (`llvm::BumpPtrAllocator`).
* **Multi-file Compilation:** Parallel 2-pass compilation pipeline using `llvm::ThreadPool`, resolving `include` directives with circular dependency detection.
* **Object Model:** Classes with `public:`, `private:`, and `protected:` access specifiers, single inheritance, and dynamic polymorphism via virtual method tables (vtables).
* **Static Analysis:** Strict typing system with safe auto-casting, context stacks for lexical scopes, and fail-fast early syntax error detection.

## Compiler Architecture

The compiler operates across a modular pipeline:

1. **Frontend:** Branchless lexical tokenization (`Lexer::tokenize`) and recursive descent instruction parsing.
2. **Pass 1 (Analysis & Registration):** AST construction and declaration registration via `FillingVisitorRegistry` and `FillingVisitorBodyClass`.
3. **Pass 2 (Environments & Emission):** Symbol table resolution (`BuilderEnvironmentRegistryFunction`, `BuilderEnvironmentRegistryVariable`) and LLVM IR emission via `GeneralVisitorGenCode`.
4. **Backend:** Optimizations and native binary generation by the LLVM infrastructure.

## Syntax

```rust
fn int32 main() {
    dec string message = "Prysma system operational";
    call print(ref message);
    return 0;
}
```

## Compilation with Docker

The official compilation toolchain (LLVM 18, Clang 18, LLD, CMake, ccache, Python) is encapsulated within a Docker container to ensure reproducible builds.

### Docker Installation

| Host OS | Installation Guide |
| :--- | :--- |
| **Arch Linux** | [Arch Linux Guide](docs/en/installation/docker/arch_linux.md) |
| **Debian** | [Debian Guide](docs/en/installation/docker/debian.md) |
| **Ubuntu** | [Ubuntu Guide](docs/en/installation/docker/ubuntu.md) |
| **Windows (PowerShell)** | [Windows Guide](docs/en/installation/docker/windows_powershell.md) |

### Quick Start

1. **Build the compilation image:**
   ```bash
   docker build --target debug -t prysma-debug -f docker/compiler/Dockerfile .
   ```

2. **Compile the project from the host:**
   ```bash
   # Release mode (-O3, -march=native, -flto, lld)
   docker run --rm -v "$(pwd)":/workspace prysma-debug python3 build.py

   # Instrumented Debug mode (-g3, -O0, ASan, UBSan)
   docker run --rm -v "$(pwd)":/workspace prysma-debug python3 debug.py
   ```

3. **Interactive session:**
   ```bash
   docker run --rm -it -v "$(pwd)":/workspace prysma-debug
   ```

The `build.py` / `debug.py` script automatically runs internal AST node, visitor, and parser generators before calling CMake and Clang.

### VS Code Integration

The provided `.vscode` configuration interfaces directly with the container:

* **Compilation (`Ctrl+Shift+B`):** Compiles the active `.p` source file via `docker/environnement_dev/linux/docker-cli.sh` (or `docker/environnement_dev/windows/docker-cli.ps1` on Windows).
* **Debugging (`F5`):** Debugger launch profiles for LLDB (C++ compiler, port 1234) and debugpy (Python generators, port 5678).
* **Clangd Code Intelligence:** C++ code navigation and completion on the host without local LLVM dependencies.
* **Syntax Highlighting:** Extension in `.vscode/SyntaxeCouleurPrysma` for `.p` files.

See the [VS Code Environment Guide](docs/en/installation/vscode_environment.md). For advanced options and benchmarks, see the [Docker Compilation Guide](docs/en/installation/docker_compilation.md).

## Documentation

Technical documentation is available in `docs/`:

* **Compilation & Environment:**
  * [Write and run your first Prysma program](docs/en/installation/first_program.md)
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
