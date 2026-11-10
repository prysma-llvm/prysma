# Centralized Architecture and Docker Compilation

This document describes Prysma's compilation architecture and the commands to build Docker images, compile the project, and work in interactive mode.

## 1. Centralized Architecture

Prysma is a system compiler requiring a complete toolchain:

* **LLVM 18** and development libraries (`llvm-18`, `llvm-18-dev`, `libclang-rt-18-dev`)
* **Clang 18** compiler and **LLD 18** linker
* **C++23**, **CMake**, and **ccache** for fast incremental compilation
* Analysis and diagnostic tooling: **Bear** (`compile_commands.json` generation) and **Graphviz**
* Native debugging: **GDB**, **LLDB-18**, and **debugpy** for Python tooling
* Dedicated Python 3 virtual environment preloaded with compiler dependencies

Manual local installation of these components frequently leads to toolchain divergence, ABI mismatches, and portability issues across different operating systems.

The official build workflow relies on **Docker as a centralized architecture**:
- **Reproducible environment**: developers, CI runners, and benchmark servers share the exact same tool versions.
- **No heavy host dependencies**: no LLVM packages or specialized libraries are required on the host system. Only Docker is needed.
- **Ready to use**: pre-configured environment variables, LLVM paths, and Python virtual environment ready for immediate use.

## 2. Prerequisites: Host Docker Installation

Refer to your operating system's guide to install and configure Docker:

* [Arch Linux Setup](docker/arch_linux.md)
* [Debian Setup](docker/debian.md)
* [Ubuntu Setup](docker/ubuntu.md)
* [Windows (PowerShell) Setup](docker/windows_powershell.md)

## 3. Building the Docker Images

> **Important:** The build command must be executed from the root of the `prysma` project so the Dockerfile can access the complete build context (`requirements.txt`, sources, etc.).

The Dockerfile is structured using a multi-stage build to produce tailored targets:

```bash
# Development and debug image (gdb, lldb-18, bear, graphviz, python venv)
docker build --target debug -t prysma-debug -f docker/compiler/Dockerfile .

# Lightweight benchmark server image (includes linux perf tools)
docker build --target benchmark -t prysma-benchmark -f docker/compiler/Dockerfile .
```

Both targets share the same base layer (Ubuntu 24.04, LLVM 18, Clang, Python), maximizing Docker layer caching during rebuilds.

## 4. Compilation Orchestration (`build.py` & `debug.py`)

Compilation inside the container is orchestrated by two Python wrappers: `build.py` and `debug.py`.

### Role of the Python Wrapper
* **Automated code generation**: Before C++ compilation begins, the script runs the compiler's code generators (`GeneratorAST`, `GeneratorInterfaceVisitor`, `GeneratorVisitorBaseGeneral`, `GeneratorGraphViz`, `GeneratorExpression`, `GeneratorParser`) to automatically generate the AST classes, visitor interfaces, and parser.
* **Dynamic configuration**: Configures `ccache` (10 GB cache limit), detects available CPU cores (`--parallel`), and dynamically injects fine-tuned compilation flags.

### Compilation Modes
* **`build.py` (Release Mode)**: Optimized build. Injects `-O3`, `-march=native`, `-ffast-math`, `-fomit-frame-pointer`, `-flto`, and utilizes the `-fuse-ld=lld` linker with dead-code elimination (`--gc-sections`).
* **`debug.py` (Debug Mode)**: Instrumented build. Injects `-g3`, `-O0`, `-fno-omit-frame-pointer`, `-fstack-protector-all`, strict warnings, and activates memory safety tooling with **AddressSanitizer** (`-fsanitize=address`) and **UndefinedBehaviorSanitizer** (`-fsanitize=undefined`).

## 5. Direct Compilation Commands from the Host

The local source directory is mounted into `/workspace` inside the container. Commands can be invoked directly from the host terminal:

```bash
# Compile Prysma (optimized release build with ccache)
docker run --rm -v "$(pwd)":/workspace prysma-debug python3 build.py

# Compile in debug mode (with Bear and injected sanitizer flags)
docker run --rm -v "$(pwd)":/workspace prysma-debug python3 debug.py

# Run test suite
docker run --rm -v "$(pwd)":/workspace prysma-debug python3 tests/run_unit_tests.py

# Execute the compiled binary
docker run --rm -v "$(pwd)":/workspace prysma-debug ./build/Prysma --help
```

> **Windows PowerShell:** Replace `"$(pwd)"` with `"${PWD}"`.

## 6. Interactive Container Session (Shell)

To work directly inside the centralized environment:

```bash
docker run --rm -it -v "$(pwd)":/workspace prysma-debug
```

Inside the container, the working directory is set to `/workspace` with LLVM 18, Clang, and the Python virtual environment preloaded in `PATH`:

```bash
# Release compilation
python3 build.py

# Or debug compilation (with sanitizers injected)
python3 debug.py

# Run unit tests
python3 tests/run_unit_tests.py

# Test the compiled executable
./build/Prysma --help

# Exit the container
exit
```

## 7. Attaching to an Active Container

If a container is already running in the background and requires debugging:

```bash
# 1. Identify the target container
docker ps

# 2. Attach an interactive bash session
docker exec -it <container_name_or_id> /bin/bash
```

## 8. Running Performance Benchmarks

To execute the benchmark suite on a specific Git branch across all host CPU cores:

```bash
docker run --rm --cap-add=SYS_ADMIN \
  -v "/tmp/prysma-bench":/prysma \
  -v "$(pwd)":/workspace \
  prysma-benchmark <branch_name>
```

## 9. Hardware Performance Counters (PMU / Linux perf)

If the benchmark runner indicates restricted access to host hardware performance counters (cycles, cache misses, branch predictions), temporarily adjust Linux kernel settings:

```bash
sudo sysctl -w kernel.perf_event_paranoid=-1
sudo sysctl -w kernel.kptr_restrict=0
```

## 10. VS Code Integration and `docker-cli.sh` Wrapper

The repository includes a root `docker-cli.sh` wrapper script and pre-configured `.vscode/tasks.json` tasks:

* **Environment detection**: Transparently detects host vs. Flatpak sandbox execution (`flatpak-spawn`).
* **Permission preservation**: Executes commands with host user UID/GID (`--user $(id -u):$(id -g)`), ensuring generated `build/` files remain accessible without `sudo`.
* **Clangd indexing**: Automatically rewrites `/workspace` paths to absolute host paths in `build/compile_commands.json` and bridges LLVM headers (`.clangd-includes`), delivering immediate IDE autocompletion and diagnostics without requiring host LLVM installation.

For instructions on configuring `.vscode/Prysma.code-profile`, packaging the syntax coloring extension, and interactive debugging, see the [VS Code Environment Guide](vscode_environment.md).
