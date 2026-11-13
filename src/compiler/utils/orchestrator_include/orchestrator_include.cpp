//===-- orchestrator_include.cpp --------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/registry/registry_function.h"
#include "compiler/ast/utils/orchestrator_include/orchestrator_include.h"
#include "compiler/ast/utils/orchestrator_include/unit_compilation.h"
#include "compiler/manager_error.h"
#include "compiler/file_processing/file_reading.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/visitor/visitor_filling_registry/visitor_filling_registry.h"
#include <iostream>
#include <llvm-18/llvm/IR/DerivedTypes.h>
#include <llvm-18/llvm/IR/Instructions.h>
#include <llvm-18/llvm/IR/Value.h>
#include <cstdlib>

#include <llvm-18/llvm/Support/ThreadPool.h>
#include <memory>
#include <mutex>
#include <string>


OrchestratorInclude::OrchestratorInclude(RegistryFunctionGlobal* registryFunctionGlobale, FileRegistry* registryFile, std::mutex* mutex, bool enableGraphViz)
    : _mutex(mutex), _registryFunctionGlobal(registryFunctionGlobale), _registryFile(registryFile), _enableGraphViz(enableGraphViz)
{}

OrchestratorInclude::~OrchestratorInclude()
= default;

auto OrchestratorInclude::isGraphVizEnabled() const -> bool {
    return _enableGraphViz;
}

auto OrchestratorInclude::hasErrors() const -> bool {
    return _hasErrors;
}

void OrchestratorInclude::compileProject(const std::string& filePath) 
{
    includeFile(filePath);
    _threads.wait(); // Wait for all threads to finish pass 1


    for(const auto& unit : _compilationUnits) {
        UnitCompilation* ptrUnit = unit.get();
        _threads.async([this, ptrUnit] {
            try {
                ptrUnit->pass2();
            } catch (const CompilationError& e) {
                std::lock_guard<std::mutex> lock(*_mutex);
                _hasErrors = true;
                std::cerr << "Error in file '" << ptrUnit->getPath() << "': " << e.what() << std::endl;
            }
        });
    }
    _threads.wait(); // Wait for all threads to finish pass 2
}

void OrchestratorInclude::includeFile(const std::string& absolutePath)
{
    std::lock_guard<std::mutex> lock(*_mutex);
    if (_alreadyIncludedFiles.count(absolutePath) > 0) {
        return; // File already included, ignore to avoid multiple inclusions
    }
    _alreadyIncludedFiles.insert(absolutePath);

   // Fill the compilation unit vector 
    _compilationUnits.push_back(std::make_unique<UnitCompilation>(this, _registryFile, absolutePath, _registryFunctionGlobal));
    auto *ptrNewUnit = _compilationUnits.back().get();

    // Fill the thread pool 
    _threads.async([this, ptrNewUnit, absolutePath] {
        try {
            ptrNewUnit->pass1();
        } catch (const CompilationError& e) {
            std::lock_guard<std::mutex> lock(*_mutex);
            _hasErrors = true;
            std::cerr << "Error in file '" << absolutePath << "': " << e.what() << std::endl;
        }
    });
}
