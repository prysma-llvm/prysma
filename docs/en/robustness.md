# Robustness and Security - Prysma Compiler

## Overview

This document details the architectural guarantees, error containment mechanisms, and concurrency synchronization controls implemented in the Prysma compiler to handle edge cases, prevent memory corruption, and ensure execution safety.

## 1. Lexical Analysis: Memory Safety (`Lexer::tokenize`)

**Component:** `Lexer::tokenize`
**Role:** Primary boundary validating untrusted raw input buffers.

### Edge Case 1: Empty Source File or Whitespace-Only Buffer

* **Scenario:** A `.p` file containing 0 bytes or composed entirely of whitespace characters (`\t`, `\n`, `\r`, spaces).
* **Handling:** The tokenization loop traverses the input without emitting statement tokens and automatically appends a single terminal `TOKEN_EOF` token.
* **Result:** The token vector is never empty (minimum size = 1). The AST builder immediately inspects `TOKEN_EOF` and generates a valid minimal compilation unit without null pointer dereferences or undefined behavior.

### Edge Case 2: Unterminated String Literal

* **Scenario:**
  ```rust
  dec string message = "String abruptly ended before EOF
  ```
* **Handling:** Upon scanning an opening quote (`"`), the lexer verifies `cursor < end` at every character advancement. If EOF is reached before a matching closing quote, a `CompilationError` exception is thrown with the exact line number.
* **Result:** Prevents buffer overreads and blocks invalid token sequences from advancing to the parsing phase.

### Edge Case 3: Fuzzing & Malformed Byte Sequences

* **Scenario:** Random binary sequences, invalid UTF-8/ASCII bytes, or corrupted file data.
* **Handling:** The precomputed 256-byte classification lookup table (`cclut_array`) maps bytes outside the valid ASCII grammar to `CC_OTHER`. The compiler emits a controlled syntax error without memory faults.
* **Verification:** An automated fuzzing test (`Tester Lexer Fuzzing - Garbage aleatoire` in `tests/unit_tests/src/compiler/lexer/test_lexer.cpp`) verifies that feeding 5,000 random unconstrained bytes cannot crash or fault the lexer.

## 2. Syntax Analysis & Expressions (`BuilderTreeEquation`)

**Component:** `BuilderTreeEquation` and `BuilderTreeInstruction`
**Role:** Abstract Syntax Tree (AST) construction from token streams.

### Edge Case 1: Asymmetric or Empty Parentheses

* **Scenario:**
  ```rust
  dec int32 x = ();
  dec int32 y = (10 + );
  dec int32 z = ((5 + 3);
  ```
* **Handling:** `ServiceParenthese` verifies delimiter symmetry and structure prior to node allocation. If an expression collapses to empty or lacks closing delimiters, a syntax exception is thrown with line context.
* **Result:** Incomplete or corrupted AST nodes are never instantiated into the memory arena.

### Edge Case 2: Consecutive Orphan Operators

* **Scenario:**
  ```rust
  dec int32 bad = 10 / / 2;
  ```
* **Handling:** The operator precedence chain (`ChainOfResponsibility`) isolates orphan operators missing operands on either side. Attempting to parse missing operands throws a controlled semantic exception rather than emitting malformed LLVM IR.

## 3. Multi-File Compilation & Concurrency (`OrchestratorInclude`)

**Component:** `OrchestratorInclude`
**Role:** Coordinates parallel translation unit compilation via `llvm::ThreadPool`.

### Edge Case 1: Circular Include Dependencies

* **Scenario:**
  ```
  file_a.p includes file_b.p
  file_b.p includes file_c.p
  file_c.p includes file_a.p  ← potential infinite cycle
  ```
* **Handling:**
  1. On each `include` directive, the resolved file path is canonicalized via `std::filesystem::absolute`.
  2. Access to `_alreadyIncludedFiles` is guarded by a mutex (`std::lock_guard<std::mutex>`).
  3. If the path already exists in the set, the inclusion request returns immediately.
  4. If the path is new, it is inserted into the set and a new `UnitCompilation` task is dispatched to the thread pool.
* **Result:** Infinite inclusion recursion and uncontrolled thread generation are eliminated in $O(\log F)$ (where $F$ is total project files).

### Edge Case 2: Exception Thrown Inside a Worker Thread

* **Scenario:** A syntax or type error occurs in a file during Pass 1 while other files compile concurrently on background threads.
* **Handling:**
  1. Each worker thread encapsulates pass execution (`unit->pass1()` or `unit->pass2()`) in a `try / catch (const CompilationError& e)` block.
  2. Upon failure, the thread acquires the orchestrator mutex, logs the error with source file context, and sets the atomic flag `_hasErrors = true`.
  3. The orchestrator calls `_threads.wait()` ensuring all running Pass 1 tasks finish cleanly.
  4. If `_hasErrors` is set, Pass 2 (LLVM IR code generation) is cancelled entirely.
* **Result:** LLVM IR is never emitted from incomplete or invalid semantic states. Memory arena resources are reclaimed cleanly without leaks.

## 4. Memory Alignment & Static Type Safety

**Components:** `ContextGenCode`, LLVM generation
**Role:** Prevents silent memory corruption during SSA IR emission.

### Edge Case: Scalar Assignments Across Incompatible Widths

* **Scenario:** Assigning a 64-bit integer into a 32-bit register, or floating-point to integer conversion without explicit truncation.
* **Handling:** Code generation requires explicit casting paths (`creerAutoCast`). Conversion instructions map directly to LLVM SSA instructions (`CreateFPTrunc`, `CreateFPExt`, `CreateSExt`, `CreateTrunc`).
* **Memory Diagnostics:** Debug builds (`python3 debug.py`) enable **AddressSanitizer (ASAN)** (`-fsanitize=address`) and **UndefinedBehaviorSanitizer (UBSan)** (`-fsanitize=undefined`), immediately detecting buffer overflows, double frees, or unaligned memory loads.

## 5. Security & Robustness Matrix

| Component | Risk / Edge Case | Protection Mechanism | Resulting Behavior |
| :--- | :--- | :--- | :--- |
| **Lexer** | Empty source file | Guaranteed insertion of `TOKEN_EOF` | Valid minimal unit without crash |
| **Lexer** | Unterminated string at EOF | Strict boundary check `cursor < end` | Immediate failure with line number |
| **Lexer** | Corrupted binary data | 256-byte LUT classification (`CC_OTHER`) | Clean rejection without memory faults |
| **Parser** | Malformed parentheses | Upfront structural integrity check | Syntax exception prior to node allocation |
| **Orchestrator** | Circular include dependencies | Mutex-guarded deduplication set | Cycle prevention without infinite recursion |
| **Orchestrator** | Thread worker failure | Atomic error tracking + synchronization barrier | Cancellation of Pass 2 (fail-fast) |
| **Backend** | Type representation mismatch | Explicit LLVM SSA casts + ASAN/UBSan | Absence of undefined behavior |
