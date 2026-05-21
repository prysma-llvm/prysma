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

 - Currently in the experimental phase. This part is incomplete and may exhibit instabilities.

## Documentation

In accordance with the project's modular structure, all technical documentation and configuration procedures are isolated in the `/Docs` directory.

  * **Configuration and Installation:** [Docs/EN/Installation/SETUP_UBUNTU.md](Docs/EN/Installation/SETUP_UBUNTU.md)
  * **Architecture Analysis:** [Docs/EN/ARCHITECTURE.md](Docs/EN/ARCHITECTURE.md)
  * **Robustness and Security Analysis:** [Docs/EN/ROBUSTNESS.md](Docs/EN/ROBUSTNESS.md)
  * **Language Specifications:** [Docs/EN/PRYSMA.md](Docs/EN/PRYSMA.md)
  * **Technical Analysis Report:** [Docs/EN/Analysis/TECHNOLOGICAL_CHOICES.md](Docs/EN/Analysis/TECHNOLOGICAL_CHOICES.md)

## License

This project is distributed under the **GPL v3** license. Consult the `LICENSE` file for legal details.

## Authors

For the list of contributors and authors, see [AUTHORS.md](AUTHORS.md).
