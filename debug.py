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

def main():
    dossier_script = os.path.dirname(os.path.abspath(__file__))
    os.chdir(dossier_script)

    GeneratorAST(dossier_script).generate()
    GeneratorInterfaceVisitor(dossier_script).generate()
    GeneratorVisitorBaseGeneral(dossier_script).generate()
    GeneratorGraphViz(dossier_script).generate()  
    GeneratorExpression(dossier_script).generate()
    GeneratorParser(dossier_script).generate()
    
    cxxflags = (
        "-fno-rtti -g3 -O0 " # Debug info max, aucune optimisation
        "-fsanitize=address -fsanitize=undefined " # Les détecteurs de bugs mémoire
        "-fstack-protector-all " # Ajout du protecteur de pile
        "-fno-omit-frame-pointer " # Pour des backtraces propres
        "-D_DEBUG " # Assertions et debug de la lib standard
        "-Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor "
        "-Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual "
        "-Wconversion -Wsign-conversion -Wnull-dereference -Wformat=2 "
        "-ffunction-sections -fdata-sections " # section pour le code mort 
    )

    # 2. Les sanitizers doivent aussi être passés au Linker
    ldflags = "-fsanitize=address -fsanitize=undefined"

    ccache_path = setup_ccache()

    cmake_args = [
        "cmake", "-S", ".", "-B", "build", 
        "-DCMAKE_CXX_COMPILER=clang++",  
        "-DCMAKE_C_COMPILER=clang",
        "-DCMAKE_BUILD_TYPE=Debug",
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

    nb_coeurs = str(os.cpu_count() or 1)

    subprocess.run([
        "cmake", "--build", "build", "--parallel", nb_coeurs
    ], check=True)

    if ccache_path:
        print("\n ccache statistics")
        subprocess.run(["ccache", "--show-stats"], check=False)
    
if __name__ == "__main__":
    main()