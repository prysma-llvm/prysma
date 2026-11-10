# Engineering Changelog - Architecture & Technical Decisions (Prysma)

## 1. Abstract Syntax Tree (AST) & Visitor Refactoring

* **Dependency Injection (General Visitor):** Initial context propagation through builders required changes across dozens of separate sites for a single feature, introducing risks of uninitialized states. Injecting `ContextGenCode` directly into the general visitor centralizes access to symbol registries and the memory arena.
* **Mutual Recursion:** The legacy expression extraction algorithm, based on manual delimiter scanning, lacked resilience when function calls were nested inside arithmetic equations. Replacing it with mutual recursion between `BuilderTreeEquation` and `BuilderTreeInstruction` stabilizes complex expression parsing.
* **Typing & Abstraction:** Encapsulating `llvm::Value*` behind the `IType*` abstraction in registries provides the compiler with exact semantic type information and target memory addresses without runtime inference.
* **Automated Node Generation:** AST node classes and visitor interfaces are generated dynamically via Jinja2 / Python templates from a declarative YAML definition (`ast.yaml`), ensuring node enumeration consistency without manual boilerplate.
* **Removal of `dynamic_cast`:** Replaced runtime dynamic casting with compile-time generated enumerated node identifiers, speeding up node classification during AST traversals.

## 2. Memory Management & LLVM

* **Arena Allocator (`llvm::BumpPtrAllocator`):** AST memory management migrated from `std::shared_ptr` to contiguous LLVM bump-pointer arenas. Raw pointers (`INode*`) allocated inside the arena maximize L1/L2 cache locality and eliminate memory leaks without recursive deallocation traversals.
* **Opaque Pointer Passing:** When passing aggregate structures, the compiler passes direct memory addresses as 64-bit opaque pointers (`ptr`), avoiding deep structure copying.
* **Arrays by Composition:** The array architecture relies on composition rather than inheritance, supporting dynamic dimensions for variable storage.
* **LLVM 18 Decompilation:** Systematically decompiling C++ code to LLVM 18 IR serves as empirical documentation to verify branch emission (`if-else`, `while` loops, byte alignments) against Clang 18 requirements.

## 3. Optimizations & Tooling Workflow

* **Toolchain (CMake, Clang 18, LLD):** Standardized on Clang 18 and the LLD linker with `ccache` integration to accelerate incremental rebuilds. Automated file discovery in `CMakeLists.txt` streamlines project structure.
* **Parallel Compilation (`llvm::ThreadPool`):** Unbounded thread spawning was replaced with LLVM's thread pool. Multi-file compilation is split into Pass 1 (lexer, AST, signatures) and Pass 2 (local environments, LLVM IR emission), preventing memory access conflicts.
* **Thread-Safe Synchronization:** Shared global registries are guarded by mutexes, and the orchestrator centralizes circular include detection via a synchronized deduplication set.
* **Continuous Instrumentation (ASAN / UBSan):** AddressSanitizer and UndefinedBehaviorSanitizer flags are automatically injected in debug mode (`debug.py`).

## 4. Documented Bug Fixes & Capabilities

* **Float Assignment Bug:** Segmentation faults during scalar assignments across distinct floating-point precisions were eliminated via explicit SSA casting paths (`creerAutoCast`).
* **Nested Argument Scope:** Nested calls (`call testArgDanIf(param - 1)`) previously corrupted the shared argument registry. Introducing context stacks (`std::stack`) isolates local variables across active frames.
* **GraphViz Node Identifier Collisions:** Replacing static node IDs with an incremental identifier stack in the GraphViz visitor prevents label collisions in deep syntax trees.
* **Function Return Storage in Variables:** Complete support for capturing function results directly into declarations: `dec <type> var = call function(...)`.
* **Immediate Numeric Constants in Arguments:** Fixed handling of raw numerical literals passed as parameters (`call function(42, 100)`), ensuring parity between variable and immediate arguments.

## 5. Lexer Algorithm & Design

* **Compile-Time Classification (LUT):** $O(1)$ ASCII character classification without conditional branches via a 256-entry table generated through template metaprogramming (`cclut_array`).
* **SWAR Keyword Resolution:** Keywords are encoded into Little-Endian 64-bit integers at compile time (`to_kw()`) and evaluated via direct unaligned memory loads (`ld16`, `ld32`, `ld64`) under a `switch(word.size())`.
* **Branchless Unary Resolution:** A 128-entry precomputed boolean table (`unary_lut`) determines unary vs binary operator semantics based on the preceding token type.

## 6. Next Engineering Priorities

* **Pratt Expression Parser:** Replacing the operator precedence chain with a Pratt parser to linearize equation tree construction costs.
* **Chained Method Calls:** Supporting consecutive method invocations on returned objects (`object.getOther().method()`).
* **External C Declarations (`extern`):** Linking standard C runtime functions without manual wrapper shims.
