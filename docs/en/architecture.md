# Architecture - Prysma Compiler

## Overview

Prysma is a compiler for a Turing-complete language that generates LLVM 18 intermediate code. The architecture follows a modular three-phase structure: lexical analysis (Lexer), Abstract Syntax Tree (AST) construction, and LLVM code generation via the Visitor pattern.

## General Project Schema

```
Prysma Source Code (.prysma)
         ↓
    [Lexer] - Tokenization (O(n))
         ↓
  Token Vector
         ↓
 [InstructionTreeBuilder] - Recursive Descent (O(t))
         ↓
 [EquationTreeBuilder] - Mutual Recursion (O(k·e²))
         ↓
   Syntax Tree (AST)
         ↓
  [GeneralCodeGenVisitor] - Visitor Pattern
         ↓
   LLVM IR Code (.ll)
         ↓
 [LLVM Backend] - Optimization & Compilation
         ↓
 Executable Machine Code
```

## Frontend Execution Flow: AST Generation

### Components and Interactions

1. **Main**: Central controller orchestrating the sequential flow without executing analysis logic.

2. **TextFileReader**: Extracts raw code from the source file and returns a `std::string`.

3. **Lexer**: Analyzes the text and generates a vector of tokens (lexical analysis).
   - Classifies keywords via `O(1) amortized` hash table
   - Processes operators via `O(1)` switch
   - Scans character by character in `O(n)` where n = number of characters

4. **InstructionTreeBuilder**: Central builder driven by recursive descent.
   - Delegates to specialized parsers via `InstructionRegistry`
   - Each token consumed only once → `O(t)` where t = number of tokens

5. **EquationTreeBuilder**: Recursively resolves mathematical expressions.
   - Uses `ResponsibilityChain` to identify the lowest precedence operator
   - Splits the left/right expression and applies recursion
   - Worst-case complexity: `O(k·e²)` where k = precedence levels (13), e = expression tokens

6. **INode**: Represents the AST logic.
   - Base interfaces: `INode`, `IInstruction`, `IExpression`
   - Constructed recursively, each node instantiating its children according to Prysma grammar

### Architectural Constraint

The generated AST is returned to Main **only if complete**. This strict isolation enforces the **Fail-Fast** principle:
- Error detected by Lexer or InstructionTreeBuilder → compilation halted immediately
- Prevents memory corruption
- Ensures LLVM receives a 100% valid syntax tree

## Detailed Algorithm Description

### Step 1: Lexical Analysis (Lexer::tokenizer)
```cpp
Lexer::tokenizer(const std::string& source) → std::vector<Token>
```
- Scans the file character by character
- Separates word by word, assigning a unique identifier to each token
- Removes superfluous elements (spaces, invalid characters)
- Result: vector of tokens with identifiers and string values

### Step 2: Instruction Tree Construction
```cpp
InstructionTreeBuilder::build(
    std::vector<Token>& tokens, int& index
) → INode
```
- Scans the token vector with a recursive descent algorithm
- For each token, retrieves the specific parser via `InstructionRegistry`
- Delegates parsing to the found parser (e.g., VariableParser, IfParser, WhileParser)
- Allows recursion by passing the current builder to the parser
- Result: complete instruction tree

### Step 3: Recursive Equation Tree Construction
```cpp
EquationTreeBuilder::build(const std::vector<Token>& equation) → INode
```

**Pseudo-code:**
```
function build(equation : TokenVector) → INode
    // Clean enclosing parentheses
    equation ← removeEnclosingParentheses(equation)
    
    // Base case: no operator found = numeric value
    index ← responsibilityChain.findOperator(equation)
    if index == -1
        return createValueNode(convertToFloat(equation[0]))
    end if
    
    // Recursive case: divide and conquer
    operator ← symbolRegistry.getNode(equation[index])
    left ← equation[0..index-1]
    right ← equation[index+1..end]
    
    // Recursive calls
    leftSubtree ← build(left)
    rightSubtree ← build(right)
    
    // Link subtrees
    operator.add(leftSubtree, rightSubtree)
    return operator
end function
```

### Step 4: LLVM Code Generation via Visitor Pattern
- Uses the `IVisitor` interface implemented by `GeneralCodeGenVisitor`
- Traverses the AST in depth `O(n)` where n = number of nodes
- Each node calls `accept(this)` to generate its LLVM IR
- Produces a `.ll` file (LLVM intermediate code)

### Step 5: LLVM Backend
- Optimizes the IR code
- Generates assembler for the target architecture
- Produces the final executable

## Design Patterns

### 1. Composite + Interpreter (AST Structure)
**Base interfaces:** `INode`, `IExpression`, `IInstruction`  
**Benefit:** Transparent manipulation of the Turing-complete grammar, strict syntax validation before compilation.

### 2. Visitor (Visitor Pattern)
**Implementations:** `GeneralCodeGenVisitor` (LLVM IR), `GeneralGraphVizVisitor` (debugging)  
**Benefit:** Separates the AST from the generation logic, allows multiple representations without altering the nodes.

### 3. Chain of Responsibility (Chain of Responsibility)
**Classes:** `ResponsibilityChain`, `OperatorHandler`  
**Benefit:** Automatically manages the 13 levels of mathematical precedence, avoids nested conditions.

### 4. Strategy (Strategy Pattern)
**Interface:** `IEquationStrategy`  
**Benefit:** Delegates analysis on the fly based on the token type (identifier, function call, etc.).

### 5. Builder (Builder Pattern)
**Classes:** `InstructionTreeBuilder`, `EquationTreeBuilder`  
**Benefit:** Centralizes the instantiation of AST nodes and LLVM API, prevents memory leaks.

### 6. Facade (Facade Pattern)
**Class:** `LlvmBackend`  
**Benefit:** Encapsulates the complexity of the C++ LLVM API behind high-level methods (`createAutoCast`, `declareExternal`, `loadValue`).

### 7. Context Object (Context Object)
**Structure:** `CodeGenContext`  
**Benefit:** Encapsulates the global compilation state (registers, backend, allocation arena), injects dependencies into the visitor without overloading signatures, prepares multi-file thread safety.

## Memory Management

### Arena Allocator (Bump Pointer)
- Migrated from `std::shared_ptr` to LLVM `BumpPtrAllocator`
- Conversion to raw pointers (`INode*`)
- Advantages:
  - Contiguous memory → L1/L2 cache optimization
  - Reduced fragmentation
  - Limited risk of memory leak
  - Performance: `O(1)` amortized allocation

### Global Registers
- `VariableRegistry`: storage for variable declarations
- `TypeRegistry`: type introspection
- `InstructionRegistry`: parser dispatching
- `FileRegistry`: inclusion management (protected by mutex)

## Implemented Optimizations

### CMake and Clang
- Replaces outdated Makefiles
- Integrates `glob` function for automatic path addition
- Improved error precision with Clang

### Multithreading with LLVM::ThreadPool
- Migrated from `std::thread` to `llvm::ThreadPool`
- Limits created threads (vs uncontrolled creation: 2000 threads for 2000 files)
- Split into two passes:
  - **Pass 1**: Lexer, Parser, register filling (file-parallelized)
  - **Pass 2**: LLVM Generation (parallelized, thread-safe registers via mutex)

### AddressSanitizer (ASAN)
- Immediate detection of memory leaks and corruptions at runtime
- Allocation flags for automatic tracing

## Compiler Outputs

**Source file:** `.prysma` (text)  
**Intermediate file:** `.ll` (LLVM IR)  
**Executable:** No extension (machine code)  
**Optional debugging:**
- AST dump to console
- AST Graphviz export (visualization)
- LLVM IR code to console

**Error handling:**
- Precise messages with line number and error type
- Compilation halted on invalid syntax

## Future Improvements

### Algorithmic Optimizations
1. **Lexer:** Replace conditional logic with DFA (Deterministic Finite Automaton)
   - Estimated gain: 10-20x faster than linear traversal

2. **Equation construction:** Replace `ResponsibilityChain` with Pratt Parser
   - Improved complexity: `O(k·e²)` → `O(e)`
   - Approach used by V8 and Clang

3. **Parenthesis removal:** Use two indexes (start/end) instead of copying the vector
   - Improved complexity: `O(e²)` → `O(e)`

4. **File reading:** Use `mmap()` to directly map the file
   - Eliminates dynamic `std::string` allocations
   - Approach used by Clang

### Multithreading
- Implement bounded thread pool based on physical core count (`std::thread::hardware_concurrency()`)
- Task queue to dispatch work
- Standard approach of systems like Ninja and Make -j
