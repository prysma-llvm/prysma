//===-- unit_compilation.h --------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef DB7C496D_6A43_4B78_B490_52A0C21C5224
#define DB7C496D_6A43_4B78_B490_52A0C21C5224

#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/ast/utils/orchestrator_include/configuration_facade_environment.h"
#include "configuration_facade_environment.h"
#include <string>
#include <memory>

class OrchestratorInclude; 
struct ContextGenCode;
class INode;
class FileRegistry;
class RegistryFunctionGlobal;

class UnitCompilation
{
private: 

    // Global data group for the compilation unit
    // Recursive orchestrator for includes, uses a mutex system to protect shared data
    OrchestratorInclude* _orchestrator;

    // Separate context for each compilation unit to avoid conflicts with nested includes
    // Note: never share, always copy this data to avoid race condition problems
    std::unique_ptr<ConfigurationFacadeEnvironment> _facadeConfigurationEnvironment;
    std::string _fileName;
    
    std::string _oldDirectory;
    std::string _currentDirectory;

    FileRegistry* _fileRegistry;
    ContextGenCode* _context;
    INode* _tree;
    std::string _originalFilePath;
    std::string _sourceDocument;

public: 
    UnitCompilation(OrchestratorInclude* orchestrator, FileRegistry* registry, std::string filePath, RegistryFunctionGlobal* registryFunctionGlobal);
    ~UnitCompilation();

    UnitCompilation(const UnitCompilation&) = delete;
    auto operator=(const UnitCompilation&) -> UnitCompilation& = delete;
    UnitCompilation(UnitCompilation&&) = delete;
    auto operator=(UnitCompilation&&) -> UnitCompilation& = delete;

    void pass1();
    // Called after all pass1 threads are finished.
    // Fills local registries (function, variable) from the complete global registry.
    void pass2();

    PRYSMA_NODISCARD auto getPath() const -> std::string;
};

#endif /* DB7C496D_6A43_4B78_B490_52A0C21C5224 */
