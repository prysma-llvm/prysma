# Technical Roadmap - Prysma Compiler

This document summarizes the current technical capabilities implemented in the Prysma compiler alongside planned engineering milestones for future iterations.

## 1. Current Technical Capabilities (Stable)

### Core Language & Syntax
* **Primitive Types:** Signed integers (`int32`, `int64`), IEEE 754 floating-point numbers (`float`), booleans (`bool`), characters (`char`), strings (`string`), opaque pointers (`ptr`), unit type (`void`).
* **Arithmetic & Logic Operations:** Scalar operators (`+`, `-`, `*`, `/`, `%`), comparisons (`==`, `!=`, `<`, `>`, `<=`, `>=`), and boolean connectors (`&&`, `||`, `!`).
* **Control Flow:**
  * Conditional branching `if-else`.
  * Iterative loops `while` and `for`.
  * No-op statement `pass;`.
* **Functions:** Recursive functions, pass-by-value, pass-by-reference (`ref`), dereferencing (`unref`), scalar return values, and storing call results directly into variables (`dec <type> v = call <name>(...);`).
* **Identifier Management:** Explicit declaration (`dec`), assignment (`aff`), and local lexical scope handling via context stacks.

### Object Model
* **Classes:** Data structure definitions encapsulating attributes and methods.
* **Access Specifiers:** Visibility modifiers `public:`, `private:`, and `protected:`.
* **Inheritance:** Single inheritance between classes (`class Child : Parent`).
* **Polymorphism:** Dynamic virtual method tables (VTable) for runtime method resolution.

### Memory Management
* **Heap Allocation:** System operators `new` and `delete` providing manual lifecycle management.
* **Internal Arena Allocation:** Contiguous bump-pointer arena (`llvm::BumpPtrAllocator`) backing AST nodes, ensuring cache locality and eliminating memory leaks without deallocation traversals.

### Infrastructure & Tooling
* **Multi-File Compilation:** Native `include "file.p";` directive with deduplication and circular dependency prevention.
* **Parallel Execution:** Two-pass pipeline orchestrated via `llvm::ThreadPool`.
* **Centralized Docker Environment:** Complete toolchain (LLVM 18, Clang 18, LLD, CMake, ccache, debugpy, LLDB) encapsulated in a reproducible container.
* **VS Code Integration:** Preconfigured profile, syntax highlighting for `.p` source files, on-the-fly compilation via `Ctrl+Shift+B`, and Clangd code intelligence without host-level dependencies.
* **Diagnostics & Debugging:** Optional AST visualization via Graphviz (`--graphviz`) and continuous sanitizer instrumentation (ASAN / UBSan) in debug builds.

## 2. Planned Engineering Milestones

### Short Term (v1.1)
* **Pratt Expression Parser:** Replacing the operator precedence chain with a Pratt parser to reduce expression resolution complexity from $O(k \cdot e^2)$ to $O(e)$.
* **C Interoperability (`extern`):** Direct declaration of external C functions without wrappers to link standard C library symbols (libc, libm).
* **Standard Data Structures (StdLib):** Native resizable vector module and contiguous buffers.

### Medium Term (v1.2)
* **String Interning Pool:** Centralizing identifiers in a deduplicated string table to allow pointer-based equality comparisons rather than lexicographic scanning.
* **Static AST Node Dispatch:** Transitioning from virtual `accept` / `visit` traversal toward static ID-based dispatch on contiguous node arrays, improving CPU branch prediction during LLVM emission.
* **Namespaces:** Explicit symbol namespacing per module to prevent identifier collisions in large codebases.

### Long Term (Vision)
* **Just-In-Time (JIT) Compilation:** In-memory direct execution for interactive testing and tooling.
* **Domain-Specific SSA Optimization Passes:** Custom LLVM IR optimization passes tailored to Prysma's language semantics.
* **Inline Assembly (`asm`):** Direct machine instruction blocks for hardware-critical low-level routines.
