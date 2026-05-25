cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

project(PrysmaPerfTests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

find_package(LLVM 18.1.3 REQUIRED CONFIG)
llvm_map_components_to_libnames(llvm_libs
    core
    support
    irreader
    bitwriter
    ${LLVM_TARGETS_TO_BUILD}
)

file(GLOB_RECURSE SOURCES_MAIN CONFIGURE_DEPENDS "../../src/*.cpp")
list(REMOVE_ITEM SOURCES_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/../../src/main.cpp")

file(GLOB_RECURSE SOURCES_GENEREES CONFIGURE_DEPENDS "../../build/generationCode/src/*.cpp")

file(GLOB SOURCES_PERF_TESTS CONFIGURE_DEPENDS "*.cpp")

set(SOURCES ${SOURCES_MAIN} ${SOURCES_GENEREES} ${SOURCES_PERF_TESTS})

add_executable(PrysmaPerfTests ${SOURCES})

target_compile_definitions(PrysmaPerfTests PRIVATE ${LLVM_DEFINITIONS})

target_include_directories(PrysmaPerfTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../build/generationCode/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
)

target_include_directories(PrysmaPerfTests SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})

target_link_libraries(PrysmaPerfTests PRIVATE ${llvm_libs})

target_compile_options(PrysmaPerfTests PRIVATE
    -O3 -march=native
    -fcolor-diagnostics
    -ferror-limit=3
    -fshow-column
    -fcaret-diagnostics
    -Wall -Wextra -Wpedantic
    -Wshadow
    -Wnon-virtual-dtor
    -Wnull-dereference
)
