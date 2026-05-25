# Version 3.12 minimum (pour que CMake détecte les nouveaux fichiers automatiquement)
cmake_minimum_required(VERSION 3.12)

# Définition de Clang comme compiler AVANT la déclaration du project
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

project(PrysmaTests)

# Configuration du standard C++20 et de l'exportation pour clangd
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# Configuration LLVM 
find_package(LLVM 18.1.3 REQUIRED CONFIG)
llvm_map_components_to_libnames(llvm_libs 
    core 
    support 
    irreader 
    bitwriter 
    ${LLVM_TARGETS_TO_BUILD}
)

# Gestion automatique des fichiers sources 

# 1. Récupération des sources du project principal
file(GLOB_RECURSE SOURCES_MAIN CONFIGURE_DEPENDS "../../src/*.cpp")

# IMPORTANT : Exclusion du main.cpp du project principal pour éviter les conflits (double function main)
list(REMOVE_ITEM SOURCES_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/../../src/main.cpp")

# 2. Récupération du code généré
file(GLOB_RECURSE SOURCES_GENEREES CONFIGURE_DEPENDS "../../build/generationCode/src/*.cpp")

# 3. Récupération des fichiers de tests locaux
file(GLOB_RECURSE SOURCES_TESTS CONFIGURE_DEPENDS "src/*.cpp")

# Fusion des listes de sources
set(SOURCES ${SOURCES_MAIN} ${SOURCES_GENEREES} ${SOURCES_TESTS})

# Ajout du point d'entrée des tests s'il est à la racine
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
endif()

# Création de l'exécutable de test
add_executable(PrysmaTests ${SOURCES})


# Configuration de la cible (PrysmaTests) 

target_compile_definitions(PrysmaTests PRIVATE ${LLVM_DEFINITIONS})

# Définition des dossiers d'en-tête (include)
target_include_directories(PrysmaTests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../build/generationCode/include
)

# Inclusion de LLVM en mode SYSTEM pour masquer ses avertissements internes
target_include_directories(PrysmaTests SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})

# Édition des liens avec les bibliothèques LLVM
target_link_libraries(PrysmaTests PRIVATE ${llvm_libs})

# Options de compilation strictes et formatage des diagnostics
target_compile_options(PrysmaTests PRIVATE
    $<$<CONFIG:Debug>:-g;-O0;-fno-omit-frame-pointer>
    $<$<NOT:$<CONFIG:Debug>>:-O2>
    -fcolor-diagnostics     
    -ferror-limit=3        
    -fshow-column            
    -fcaret-diagnostics    
    -Wall -Wextra -Wpedantic 
    -Wshadow                
    -Wnon-virtual-dtor      
    -Wnull-dereference      
)