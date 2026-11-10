## Command to decompile a C file to LLVM IR, Clang version 18
```bash
    clang-18 -S -emit-llvm -O0 -fno-discard-value-names test.c -o test.ll
```
