# Prysma Language Specification

## 1. Principles & General Syntax

Prysma is a system compiled programming language targeting LLVM 18. It provides explicit memory management without a garbage collector, strict static typing, and native multi-file compilation.

* **Statement Termination:** Every statement ends with a semicolon (`;`).
* **Code Blocks:** Function bodies, classes, and control structures are delimited by curly braces `{ }`.
* **Case Sensitivity:** Identifiers are strictly case-sensitive.
* **Comments:** Single-line comments begin with `//`.

## 2. Multi-File Compilation (`include`)

Module and header inclusions use the native `include` directive:

```rust
include "math/geometry.p";
include "tools/validation.p";
```

The compiler resolves relative paths relative to the current source file, deduplicates inclusions, and detects circular inclusion dependencies.

## 3. Type System

| Type | Size (bits) | LLVM IR Representation | Description |
| :--- | :--- | :--- | :--- |
| `int32` | 32 | `i32` | Standard signed 32-bit integer |
| `int64` | 64 | `i64` | Signed 64-bit integer |
| `float` | 32 | `float` | Single-precision IEEE 754 floating-point |
| `bool` | 1 | `i1` | Boolean value (`true`, `false`) |
| `char` | 8 | `i8` | ASCII character |
| `string` | Variable | Memory buffer | Text character sequence |
| `ptr` | 64 | `ptr` | Opaque memory pointer |
| `void` | 0 | `void` | Unit type (function returns only) |
| `T[]` | Variable | Contiguous pointer | Array of type `T` elements (e.g. `int32[]`) |

## 4. Declarations & Assignments

* **Variable Declaration (`dec`):**
  ```rust
  dec int32 counter = 0;
  dec float pi = 3.14159;
  dec bool active = true;
  ```
* **Assignment (`aff`):**
  Modifying an existing variable requires the `aff` keyword:
  ```rust
  aff counter = counter + 1;
  ```

## 5. Functions

Functions are declared with the `fn` keyword. Arguments are defined with the `arg` prefix. Function invocation strictly requires the `call` keyword.

### Definition & Invocation

```rust
fn int32 computeSum(arg int32 a, arg int32 b) {
    return a + b;
}

fn int32 main() {
    dec int32 total = call computeSum(10, 25);
    return 0;
}
```

### Pass-by-Reference (`ref`) and Dereferencing (`unref`)

To eliminate buffer copies or provide mutable references:

```rust
fn void printMessage(arg string[] text) {
    call print(ref text);
}
```

## 6. Control Structures

### Conditional Branching (`if` / `else`)

```rust
if (value >= 0) {
    call handlePositive(value);
} else {
    call handleNegative(value);
}
```

### `while` Loop

```rust
dec int32 index = 0;
while (index < 10) {
    call display(index);
    aff index = index + 1;
}
```

### `for` Loop

```rust
for (dec int32 i = 0; i < limit; aff i = i + 1) {
    call execute(i);
}
```

### No-Op Statement (`pass`)

The `pass;` statement acts as a no-operation instruction, explicit for empty branches:

```rust
if (condition) {
    pass;
}
```

## 7. Heap Memory Management

Dynamic memory allocation and deallocation are manually controlled:

* **`new`:** Allocates a heap memory block for an object or dynamic array.
* **`delete`:** Deallocates previously acquired memory blocks.

```rust
dec ptr instance = new MyClass();
// Operate on instance
delete instance;
```

## 8. Object Model (Classes)

Classes encapsulate attributes and methods within explicit visibility sections:

```rust
class Entity {
    private:
        dec int32 id;
    public:
        fn void setId(arg int32 newId) {
            aff id = newId;
        }
        fn int32 getId() {
            return id;
        }
}
```

### Inheritance

A class may derive from a parent base class:

```rust
class Player : Entity {
    private:
        dec int32 score;
    public:
        fn void addScore(arg int32 points) {
            aff score = score + points;
        }
}
```

Supported visibility labels:
* `public:` : Members accessible from any context.
* `private:` : Members confined to the enclosing class (default visibility).
* `protected:` : Members accessible to the enclosing class and derived classes.

## 9. Operators

| Category | Symbols | Example |
| :--- | :--- | :--- |
| **Arithmetic** | `+`, `-`, `*`, `/`, `%` | `a + b * c` |
| **Comparison** | `==`, `!=`, `<`, `>`, `<=`, `>=` | `if (x != y)` |
| **Logical** | `&&`, `||`, `!` | `if (valid && !blocked)` |
| **Assignment** | `aff` | `aff x = 5;` |
| **Declaration** | `dec` | `dec int32 x = 5;` |
| **Reference** | `ref` | `call func(ref data);` |
| **Scope Resolution** | `::` | `Module::method` |
