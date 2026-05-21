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
    dossier_script = os.path.dirname(os.path.abspath(__file__))
    os.chdir(dossier_script)

    #GeneratorAST(dossier_script).generate() # NOTE: do not uncomment until the Jinja2 system is adapted to the new DoD architecture
    GeneratorInterfaceVisitor(dossier_script).generate()
    #GeneratorVisitorBaseGeneral(dossier_script).generate() # NOTE: same thing here
    #GeneratorGraphViz(dossier_script).generate() # NOTE: same thing here
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

    subprocess.run([
        "cmake", "-S", ".", "-B", "build", 
        "-DCMAKE_BUILD_TYPE=Debug",
        f"-DCMAKE_CXX_FLAGS={cxxflags}",
        f"-DCMAKE_EXE_LINKER_FLAGS={ldflags}", 
        f"-DCMAKE_MODULE_LINKER_FLAGS={ldflags}",
        f"-DCMAKE_SHARED_LINKER_FLAGS={ldflags}",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
    ], check=True)

    os.makedirs("build", exist_ok=True)
    shutil.rmtree(os.path.join("build", "obj"), ignore_errors=True)

    nb_coeurs = str(os.cpu_count() or 1)

    subprocess.run([
        "cmake", "--build", "build", "--parallel", nb_coeurs
    ], check=True)
    
if __name__ == "__main__":
    main()