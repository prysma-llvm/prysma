import os
import shutil
import subprocess
from generation.generator_ast import GeneratorAST
from generation.generator_interface_visitor import GeneratorInterfaceVisitor
from generation.generator_visitor_base_general import GeneratorVisitorBaseGeneral
from generation.generator_graphe_viz import GeneratorGraphViz
from generation.generator_expression import GeneratorExpression
from generation.generator_parser import GeneratorParser

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)

    GeneratorAST(script_dir).generate()
    GeneratorInterfaceVisitor(script_dir).generate()
    GeneratorVisitorBaseGeneral(script_dir).generate()
    GeneratorGraphViz(script_dir).generate()
    GeneratorExpression(script_dir).generate()
    GeneratorParser(script_dir).generate()

    cxxflags_list = [
        "-O3",                  # Raw speed (maximum optimization)
        "-march=native",        # Fully exploit your CPU's instructions
        "-ffast-math",          # Aggressive math calculations
        "-fno-rtti",            # No runtime type information (RTTI off)
        "-fomit-frame-pointer", # Free up a CPU register
        "-flto",                # Link Time Optimization (LTO)
        "-DNDEBUG",             # Completely disable assertions
        "-frandom-seed=42",     # Prevents changing the seed when compiling functions that use randomness for binary generation
        "-gno-record-gcc-switches",           # Removes build flags from debug symbols
        f"-fdebug-prefix-map={script_dir}=."  # Replaces the absolute path with '.'
    ]
    
    ldflags_list = [
        "-flto",
        "-Wl,--gc-sections",
        "-Wl,-s",
        "-fuse-ld=lld",
        "-Wl,--build-id=none",                # Removes the random hash from the binary
        "-Wl,--sort-sections=name",           # Deterministic sorting of sections
        "-Wl,--hash-style=sysv"               # Stable hash style
    ]

    cxxflags = " ".join(cxxflags_list)
    ldflags = " ".join(ldflags_list)

    subprocess.run([
        "cmake", "-S", ".", "-B", "build", 
        "-DCMAKE_BUILD_TYPE=Release",
        f"-DCMAKE_CXX_FLAGS={cxxflags}",
        f"-DCMAKE_EXE_LINKER_FLAGS={ldflags}", 
        f"-DCMAKE_MODULE_LINKER_FLAGS={ldflags}",
        f"-DCMAKE_SHARED_LINKER_FLAGS={ldflags}",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
    ], check=True)

    os.makedirs("build", exist_ok=True)
    shutil.rmtree(os.path.join("build", "obj"), ignore_errors=True)

    num_cores = str(os.cpu_count() or 1)

    subprocess.run([
        "cmake", "--build", "build", "--parallel", num_cores
    ], check=True)
    
if __name__ == "__main__":
    main()



    ## 2. Le "Saut Quantique" : Désactiver l'Hyper-Threading (SMT)