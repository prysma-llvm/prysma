## Commande pour décompiler un fichier C en LLVM IR, version 18 de Clang
```bash
    clang-18 -S -emit-llvm -O0 -fno-discard-value-names test.c -o test.ll
```
