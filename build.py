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

    #GeneratorAST(script_dir).generate() # NOTE: do not uncomment until the Jinja2 system is adapted to the new DoD architecture
    shutil.copytree("TEMPORAIRE/generationCode/include/compiler/ast", "build/generationCode/include/compiler/ast", dirs_exist_ok=True)
    
    GeneratorInterfaceVisitor(script_dir).generate()
    
    #GeneratorVisitorBaseGeneral(script_dir).generate() # NOTE: same thing here
    os.makedirs("build/generationCode/include/compiler/visitor", exist_ok=True)
    os.makedirs("build/generationCode/src/compiler/visitor", exist_ok=True)
    shutil.copy("TEMPORAIRE/generationCode/include/compiler/visitor/visitor_base_generale.h", "build/generationCode/include/compiler/visitor/visitor_base_generale.h")
    shutil.copy("TEMPORAIRE/generationCode/src/compiler/visitor/visitor_base_generale.cpp", "build/generationCode/src/compiler/visitor/visitor_base_generale.cpp")
    
    #GeneratorGraphViz(script_dir).generate() # NOTE: same thing here
    shutil.copytree("TEMPORAIRE/generationCode/include/compiler/visitor/ast_graph_viz", "build/generationCode/include/compiler/visitor/ast_graph_viz", dirs_exist_ok=True)
    shutil.copytree("TEMPORAIRE/generationCode/src/compiler/visitor/ast_graph_viz", "build/generationCode/src/compiler/visitor/ast_graph_viz", dirs_exist_ok=True)
    
    GeneratorExpression(script_dir).generate()
    GeneratorParser(script_dir).generate()

    cxxflags_list = [
        "-O3",                  # Raw speed (maximum optimization)
        "-march=native",        # Fully exploit your CPU's instructions
        "-ffast-math",          # Aggressive math calculations
        # A RÉACTIVER "-fno-rtti",            # No runtime type information (RTTI off) -> Afin d'assurer la compatibilité avec l'ABI de LLVM
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