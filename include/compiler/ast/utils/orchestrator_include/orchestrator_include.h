//===-- orchestrator_include.h ----------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef D2577958_A8A8_4878_AFA0_2B3478129911
#define D2577958_A8A8_4878_AFA0_2B3478129911

#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/ast/utils/orchestrator_include/unit_compilation.h"
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <set>
#include <memory>
#include <llvm-18/llvm/Support/ThreadPool.h>

struct ContextGenCode;
class INode;
class RegistryFunctionGlobal;
class FileRegistry;

class OrchestratorInclude
{
private: 
   std::mutex* _mutex;
   RegistryFunctionGlobal* _registryFunctionGlobal;
   FileRegistry* _registryFile;
   llvm::ThreadPool _threads;
   std::vector<std::unique_ptr<UnitCompilation>> _compilationUnits;
      
   // A registry dedicated to source files (.p)
   std::set<std::string> _alreadyIncludedFiles; 

   // Thread-safe error indicator to know if compilation failed
   std::atomic<bool> _hasErrors{false};

   // Flag to enable AST graph generation
   bool _enableGraphViz;

public:
    OrchestratorInclude(RegistryFunctionGlobal* registryFunctionGlobal, FileRegistry* registryFile, std::mutex* mutex, bool enableGraphViz = false);
    ~OrchestratorInclude();

    OrchestratorInclude(const OrchestratorInclude&) = delete;
    auto operator=(const OrchestratorInclude&) -> OrchestratorInclude& = delete;
    OrchestratorInclude(OrchestratorInclude&&) = delete;
    auto operator=(OrchestratorInclude&&) -> OrchestratorInclude& = delete;
    
   void compileProject(const std::string& filePath);
   void includeFile(const std::string& absolutePath);
   static void waitForPassEnd(llvm::ThreadPool& threads);
   PRYSMA_NODISCARD auto hasErrors() const -> bool;
   PRYSMA_NODISCARD auto isGraphVizEnabled() const -> bool;
};

#endif /* D2577958_A8A8_4878_AFA0_2B3478129911 */
