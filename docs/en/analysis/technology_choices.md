# Technical Analysis: Prysma Project

This document details the architectural evaluation of the engineering decisions made for the Prysma compiler, specifically regarding backend selection and code generation strategies.

## 1. Technical Objectives

Prysma aims to deliver a system language featuring deterministic execution and memory management without a garbage collector. Three code generation strategies were analyzed:

* **LLVM IR (Selected Approach):** Intermediate Representation structured in Single Static Assignment (SSA) form. Provides direct access to LLVM's advanced optimization pipelines (vectorization, dead code elimination, inlining) and native cross-compilation targeting x86-64, ARM64, and WebAssembly.
* **Manual x86-64 Backend:** Offers total control over emitted machine instructions, but requires manual register allocation (graph coloring, linear scan), stack frame alignment, and strict adherence to the System V AMD64 ABI calling conventions. Maintenance overhead heavily outweighs the granular control gained.
* **C Source Transpilation:** Simplifies initial bootstrapping, but relinquishes low-level memory layout control and impairs the ability to emit exact debugging metadata via DWARF.

### Target LLVM IR Example (LLVM 18)

LLVM 18 enforces the opaque pointer model (`ptr`), eliminating typed pointer distinctions in the intermediate representation:

```llvm
%1 = load i32, ptr %ptr_x         ; loads 32-bit integer from opaque address %ptr_x
%result = add i32 %1, 5           ; adds 5 to the loaded value
```

## 2. Machine Code Generation Challenges

### Constraints of a Custom Assembly Backend

1. **CPU Register Allocation:** The x86-64 architecture provides 16 general-purpose registers. Conflict-free register allocation requires complex algorithms (stack spilling, live-range analysis).
2. **Calling Conventions (ABI):** Manual implementation of argument passing registers (`rdi`, `rsi`, `rdx`, etc.), 16-byte stack boundary alignment, and stack frame lifecycle management.
3. **Target-Specific Optimizations:** Pipeline scheduling and vector instruction set utilization (AVX, NEON) would require micro-architecture-specific rewrites.

### Advantages of the LLVM Infrastructure

* **Target Portability:** Prysma's frontend emits target-agnostic IR; the LLVM backend adapts machine code for the target architecture.
* **Proven Optimizations:** Direct access to standard constant folding, instruction simplification, and loop vectorization passes.
* **Tooling Ecosystem:** Native compatibility with the LLD linker, GDB/LLDB debuggers, and hardware performance profilers.

## 3. Technical References

* **Rust:** Systems language compiling via LLVM, serving as a model for deterministic memory management and SSA optimizations.
* **Kaleidoscope (LLVM Tutorial):** Didactic reference provided by the LLVM team outlining AST transformation to `IRBuilder` constructs.
* **Zig:** Systems programming language emphasizing zero hidden control flow, explicit memory allocators, and reproducible toolchains.
