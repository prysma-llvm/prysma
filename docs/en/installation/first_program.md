# Your first Prysma program

This guide starts from the Prysma repository and shows how to compile and run a `.p` file.

## Build the compiler

You need [Docker](docker_compilation.md) and Git. In a terminal, clone the repository and build the development image:

```sh
git clone https://github.com/prysma-llvm/prysma.git
cd prysma
docker build --target debug -t prysma-debug -f docker/compiler/Dockerfile .
```

From the repository root, open a shell in the container. The command mounts your local checkout at `/workspace`.

**Bash (Linux or WSL)**

```sh
docker run --rm -it --user "$(id -u):$(id -g)" -e HOME=/tmp -v "$PWD:/workspace" prysma-debug bash
```

**Windows PowerShell** with Docker Desktop in Linux containers mode

```powershell
docker run --rm -it -v "$($PWD.Path):/workspace" prysma-debug bash
```

In the container, run `python3 debug.py` to build `build/Prysma`. Skip this step if you already built the compiler.

## Write and run the program

In your editor, create `hello.p` at the root of the repository:

```prysma
fn int32 main() {
    dec int32 result = 40 + 2;
    call print(result);
    call backSlashN();
    return 0;
}
```

`main` is the entry point. `print` displays the result, `backSlashN()` writes a newline, and `return 0` reports successful completion.

Back in the container, run:

```sh
./build/Prysma hello.p && ./build/hello
```

The first command compiles the source file. The second runs the executable at `build/hello`. You should see:

```text
42
```

Next: [language specification](../prysma.md).
