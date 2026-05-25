cmake_minimum_required(VERSION 3.12)

project(Prysma)

# setup C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# so my editor understands the code and underlines errors directly
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# by default, compile in Debug to see what we're doing
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE not set -> default configuration: Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# LLVM
# let cmake handle finding llvm and its paths automatically
find_package(LLVM 18.1.3 REQUIRED CONFIG)
llvm_map_components_to_libnames(llvm_libs 
    core 
    support 
    irreader 
    bitwriter 
    ${LLVM_TARGETS_TO_BUILD}
)

# my source files
# get all my .cpp files (CONFIGURE_DEPENDS avoids having to wipe the build folder when adding a file)
file(GLOB_RECURSE SOURCES_LOCALES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE SOURCES_GENEREES CONFIGURE_DEPENDS "build/generationCode/src/*.cpp")

# merge everything
set(SOURCES ${SOURCES_LOCALES} ${SOURCES_GENEREES})

# in case main.cpp is missed
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
endif()

# create the executable
add_executable(Prysma ${SOURCES})

# add llvm definitions
target_compile_definitions(Prysma PRIVATE ${LLVM_DEFINITIONS})

# my include directories
target_include_directories(Prysma PRIVATE 
    ${CMAKE_SOURCE_DIR}/include 
    ${CMAKE_SOURCE_DIR}/build/generationCode/include
)

# include llvm as SYSTEM so clang stops complaining about their 1000 warnings
target_include_directories(Prysma SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})

# link llvm libs
target_link_libraries(Prysma PRIVATE ${llvm_libs})

# my compile flags
target_compile_options(Prysma PRIVATE
    # no optimization in Debug, max optimization in Release
    $<$<CONFIG:Debug>:-g;-O0;-fno-omit-frame-pointer;-fstandalone-debug>
    $<$<NOT:$<CONFIG:Debug>>:-O2>
    
    # to read errors without a headache (colors, 3 errors max, small caret)
    -fcolor-diagnostics     
    -ferror-limit=3        
    -fshow-column            
    -fcaret-diagnostics    
    
    # strict analysis for clean code
    -Wall -Wextra -Wpedantic 
    -Wshadow                
    -Wnon-virtual-dtor      
    -Wnull-dereference      
)