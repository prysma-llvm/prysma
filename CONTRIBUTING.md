# Contribution Guide: Prysma Compiler

Prysma's development relies on strict architectural discipline. The primary goal is to maintain a deterministic, reliable compiler. Every contribution must adhere to these engineering standards.

This document outlines the requirements for contributing changes.

## 1. Reporting Issues

Before submitting an issue, ensure the problem has not already been documented. Bug reports must be factual and reproducible.

Your report must include:
* **Minimal Prysma source code** reproducing the defect (`.p` extension).
* **Execution environment** (LLVM version, OS or container environment).
* **Diagnostic logs:** If a crash occurs during compilation, provide the LLVM IR output. For runtime memory errors, provide AddressSanitizer (ASAN) or debugger (GDB/LLDB) backtraces.

## 2. Development Standards (C++)

The compiler core is written in C++23. The following rules are mandatory:

* **Separation of Concerns:** Maintain strict decoupling between Frontend (Lexer/Parser) and Backend (LLVM).
* **Metaprogramming:** Do not construct AST nodes by hand. Any new instruction or expression must be registered via the code generation pipeline (`/generation`).
* **Memory Management:** Compiler AST node allocation is managed by contiguous arenas (`llvm::BumpPtrAllocator`). Standard smart pointers (`std::shared_ptr`) are prohibited in the AST critical path to preserve cache locality. No memory leaks are tolerated.
* **Design Patterns:** Code generation modifications must implement the **Visitor** pattern interface.

## 3. Submission Protocol (Pull Requests)

1. Create a fork and develop on a dedicated branch (`feat/feature-name` or `fix/bug-name`).
2. Write clear, structured commit messages in English (e.g., `feat: Implement string concatenation in LLVM backend` or `fix: Resolve memory leak in VTable generation`).
3. Code must compile without any compiler warnings.
4. **Mandatory Verification:** You must execute the full test suite (unit and integration tests) prior to submission:
   ```bash
   python3 debug.py
   ```