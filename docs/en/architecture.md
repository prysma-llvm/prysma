# Prysma Compiler Architecture

## Overview

Prysma is a system compiler targeting the LLVM 18 infrastructure. Its architecture is divided into decoupled stages: lexical analysis driven by compile-time precomputed lookup tables, recursive descent syntax parsing, semantic verification and symbol resolution across a two-pass pipeline, and Intermediate Representation (LLVM IR) code generation in Single Static Assignment (SSA) form.

## Compiler Execution Flow

```
Prysma Source File (.p)
           ↓
     [FileReading] - Source buffer loading
           ↓
  [Lexer::tokenize] - Compile-time LUTs + SWAR (O(n))
           ↓
      Token Vector
           ↓
[BuilderTreeInstruction::build] - Recursive descent (Pass 1)
           ↓
  [BuilderTreeEquation] - Expression resolution
           ↓
  Abstract Syntax Tree (AST allocated in BumpPtr arena)
           ↓
[FillingVisitorRegistry & FillingVisitorBodyClass] - Declaration inspection
           ↓
[BuilderEnvironmentRegistryFunction & Variable] - Symbol tables (Pass 2)
           ↓
  [GeneralVisitorGenCode] - LLVM IR emission (.ll)
           ↓
 [LLVM Backend / LLD] - Optimizations & Machine code generation
           ↓
   Native Executable Binary
```

## Frontend & Compilation Pipeline

### 1. Source Ingestion (`FileReading`)

The `FileReading` component retrieves the raw text of the `.p` source file from its resolved path and loads the character buffer into memory.

### 2. Lexical Analysis (`Lexer::tokenize`)

The `Lexer::tokenize(const std::string& sourceCode)` method transforms the character stream into a sequence of `Token` structures:

* **Character Classification (256-byte LUT):** A compile-time precomputed table (`cclut_array`), generated via template metaprogramming using `std::index_sequence` (`make_cclut`), maps each ASCII byte (`CharClass`) in $O(1)$ without conditional branches.
* **Fast Memory Loads (SWAR):** Unaligned direct memory load routines (`ld16`, `ld32`, `ld64`) fetch 2-, 4-, or 8-byte blocks into CPU registers via `std::memcpy`.
* **Compile-Time Keyword Encoding (`to_kw`):** The `constexpr Lexer::to_kw()` function packs up to 8 ASCII characters into a Little-Endian 64-bit integer (`val |= (c << (8 * i))`).
* **Hash-Free Keyword Resolution:** `Lexer::resolveKeyword` branches on `switch(word.size())` and matches the 64-bit integer against compile-time constants in a single processor cycle.
* **Unary Operator Resolution (`unary_lut`):** A 128-entry compile-time boolean table determines in $O(1)$ whether operators (`+`, `-`) operate in unary or binary contexts based on the preceding token, eliminating branch mispredictions.
* **Memory Pre-allocation:** The token vector pre-allocates memory via `tokens.reserve((sourceCode.size() / 3) + 4)` to avoid heap reallocations.
* **Deterministic End-of-File:** A terminal `TOKEN_EOF` token is appended to the stream.

### 3. Abstract Syntax Tree Construction (AST)

The `BuilderTreeInstruction::build(std::vector<Token>& tokens)` method handles sequential instruction parsing:

* **Recursive Descent:** Each instruction consumes its expected tokens and delegates node construction to specialized parsers registered in `RegistryInstruction` (`ParserVariable`, `ParserIf`, `ParserWhile`, `ParserClass`, etc.).
* **Expressions and Precedence (`BuilderTreeEquation`):** Arithmetic and boolean expressions are resolved by an operator precedence chain (`ChainOfResponsibility`) which identifies the operator of lowest precedence to divide the expression into left and right subtrees.

### 4. Two-Pass Compilation Pipeline (`UnitCompilation`)

Each compilation unit is processed across two distinct passes to support out-of-order function/class declarations and cross-file symbol dependencies:

#### Pass 1 (`UnitCompilation::pass1`)
1. Path resolution and generation of a unique unit identifier (`<parent_directory>_<filename>`).
2. Lexical tokenization via `Lexer::tokenize`.
3. AST construction via `builderTreeInstruction->build(tokens)`.
4. AST traversal via `FillingVisitorRegistry` to register global function signatures in `RegistryFunctionGlobal`.
5. AST traversal via `FillingVisitorBodyClass` to inspect class definitions, attributes, and methods.

#### Pass 2 (`UnitCompilation::pass2`)
1. Allocation and population of local variable and function symbol environments via `BuilderEnvironmentRegistryFunction` and `BuilderEnvironmentRegistryVariable`.
2. AST traversal via `GeneralVisitorGenCode` to emit LLVM IR in SSA form.
3. Optional AST visualization via `GeneralVisitorGraphViz` producing `.dot` and `.svg` files when `--graphviz` is passed.
4. Writing the generated LLVM module (`.ll`) to disk via `LlvmSerializer`.

## Internal Memory Management

### Arena Allocator (`llvm::BumpPtrAllocator`)

To guarantee data cache locality and eliminate allocation overhead:

* AST nodes (`INode*`) are allocated within contiguous memory arenas (`llvm::BumpPtrAllocator`).
* Allocation requires advancing a bump pointer, operating in $O(1)$.
* Standard reference-counted smart pointers (`std::shared_ptr`) are avoided across the AST critical path.
* The entire AST memory of a translation unit is reclaimed simultaneously upon compilation context destruction, eliminating memory leaks without recursive deallocation traversals.

## Multi-File Compilation & Concurrency

Multi-file compilation is coordinated by `OrchestratorInclude`:

* **Thread Pool (`llvm::ThreadPool`):** Parallelizes Pass 1 and Pass 2 across available CPU hardware threads to prevent resource starvation.
* **Include Resolution (`include`):** When an `include "path/file.p";` directive is parsed, `OrchestratorInclude::includeFile()` resolves the target path.
* **Cycle and Duplicate Prevention:** A mutex-guarded `std::set<std::string> _alreadyIncludedFiles` ensures that each file is compiled only once.
* **Fail-Fast Error Handling:** `CompilationError` exceptions thrown within worker threads are caught, logged with the offending file and line, and flag the atomic `_hasErrors` state. If any file fails during Pass 1, Pass 2 execution is cancelled.

## Compiler Outputs

* **Source file:** `.p`
* **Intermediate representation:** `.ll` (textual LLVM IR) stored in `programme/`
* **Optional visualization:** `.dot` and `.svg` stored in `graphe/`
* **Native executable binary:** Linked via Clang / LLD from generated `.ll` modules.

## Technical Perspectives

1. **Pratt Expression Parser:** Migrating expression parsing from the current chain of responsibility to a Pratt parser to reduce complexity from $O(k \cdot e^2)$ to $O(e)$.
2. **Static AST Dispatch:** Transitioning from virtual visitor dispatch (`accept` / `visit`) toward static ID-based dispatch on contiguous node registries, improving branch prediction during code generation.
3. **String Interning Pool:** Storing source identifiers in a deduplicated string pool to allow pointer-based equality comparisons rather than string scans.
