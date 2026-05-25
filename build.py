import os
import filecmp
import shutil
import subprocess
from generation.generator_ast import GeneratorAST
from generation.generator_interface_visitor import GeneratorInterfaceVisitor
from generation.generator_visitor_base_general import GeneratorVisitorBaseGeneral
from generation.generator_graphe_viz import GeneratorGraphViz
from generation.generator_expression import GeneratorExpression
from generation.generator_parser import GeneratorParser


def setup_ccache():
    ccache_path = shutil.which("ccache")
    if not ccache_path:
        print("[WARNING] ccache is not installed. Install it with: sudo apt-get install ccache")
        print("[WARNING] Building without ccache (slower rebuilds).")
        return None

    # Configure ccache: 10 GB max size
    subprocess.run(["ccache", "--max-size", "10G"], check=True)
    subprocess.run(["ccache", "--set-config", "compiler_check=content"], check=True)
    print(f"[OK] ccache enabled ({ccache_path}), cache limit: 10 GB")
    return ccache_path


def smart_copy_file(src, dst):
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    if os.path.exists(dst) and filecmp.cmp(src, dst, shallow=False):
        return  # Content identical, skip copy to preserve timestamp
    shutil.copy2(src, dst)


def smart_copy_tree(src, dst):
    for dirpath, dirnames, filenames in os.walk(src):
        rel_dir = os.path.relpath(dirpath, src)
        dst_dir = os.path.join(dst, rel_dir)
        os.makedirs(dst_dir, exist_ok=True)
        for filename in filenames:
            src_file = os.path.join(dirpath, filename)
            dst_file = os.path.join(dst_dir, filename)
            smart_copy_file(src_file, dst_file)


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)

    GeneratorAST(script_dir).generate() 
    GeneratorInterfaceVisitor(script_dir).generate()
    GeneratorVisitorBaseGeneral(script_dir).generate() 
 
    #GeneratorGraphViz(script_dir).generate() # NOTE: same thing here
    smart_copy_tree("TEMPORAIRE/generationCode/include/compiler/visitor/ast_graph_viz", "build/generationCode/include/compiler/visitor/ast_graph_viz")
    smart_copy_tree("TEMPORAIRE/generationCode/src/compiler/visitor/ast_graph_viz", "build/generationCode/src/compiler/visitor/ast_graph_viz")
    
    GeneratorExpression(script_dir).generate()
    GeneratorParser(script_dir).generate()

    cxxflags_list = [
        "-O3",                  # Raw speed (maximum optimization)
        "-march=native",        # Fully exploit your CPU's instructions
        "-ffast-math",          # Aggressive math calculations
        "-fno-rtti",            # No runtime type information (RTTI off) -> Afin d'assurer la compatibilité avec l'ABI de LLVM
        "-fomit-frame-pointer", # Free up a CPU register
        "-flto",                # Link Time Optimization (LTO)
        "-DNDEBUG"              # Completely disable assertions
    ]
    
    ldflags_list = [
        "-flto",
        "-Wl,--gc-sections",
        "-Wl,-s",
        "-fuse-ld=lld"
    ]

    cxxflags = " ".join(cxxflags_list)
    ldflags = " ".join(ldflags_list)

    ccache_path = setup_ccache()

    cmake_args = [
        "cmake", "-S", ".", "-B", "build", 
        "-DCMAKE_CXX_COMPILER=clang++",  
        "-DCMAKE_C_COMPILER=clang",
        "-DCMAKE_BUILD_TYPE=Release",
        f"-DCMAKE_CXX_FLAGS={cxxflags}",
        f"-DCMAKE_EXE_LINKER_FLAGS={ldflags}", 
        f"-DCMAKE_MODULE_LINKER_FLAGS={ldflags}",
        f"-DCMAKE_SHARED_LINKER_FLAGS={ldflags}",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
    ]

    if ccache_path:
        cmake_args.append("-DCMAKE_CXX_COMPILER_LAUNCHER=ccache")
        cmake_args.append("-DCMAKE_C_COMPILER_LAUNCHER=ccache")

    subprocess.run(cmake_args, check=True)

    os.makedirs("build", exist_ok=True)
    shutil.rmtree(os.path.join("build", "obj"), ignore_errors=True)

    num_cores = str(os.cpu_count() or 1)

    subprocess.run([
        "cmake", "--build", "build", "--parallel", num_cores
    ], check=True)

    if ccache_path:
        print("\n ccache statistics")
        subprocess.run(["ccache", "--show-stats"], check=False)
    
if __name__ == "__main__":
    main()