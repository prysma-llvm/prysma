//===-- configuration_facade_environment.h ----------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef D8FAA486_F5BA_43FB_BFFC_AB9990B46458
#define D8FAA486_F5BA_43FB_BFFC_AB9990B46458

#include "compiler/ast/registry/data/id_generator.hpp"
#include "compiler/ast/registry/data/node_data_registry.hpp"
#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/macros/prysma_maybe_unused.h"
#include "compiler/ast/registry/context_gen_code.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/ast/registry/context_parser.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/ast/registry/stack/return_context_compilation.h"
#include "compiler/ast/registry/registry_argument.h"
#include "compiler/ast/registry/registry_expression.h"
#include "compiler/ast/registry/registry_class.h"
#include "compiler/ast/builder_tree_instruction.h"
#include "compiler/ast/registry/registry_function.h"
#include "compiler/ast/registry/registry_instruction.h"
#include "compiler/ast/registry/registry_type.h"
#include "compiler/builder/equation/builder_equation_flottante.h"
#include "compiler/parser/parser_type.h"
#include "compiler/llvm/llvm_backend.h"
#include "compiler/registry/registry_file.h"
#include <llvm/Support/Allocator.h>
#include <memory>
#include <string>

class BuilderFloatEquation;
class TypeParser;
class ConfigurationFacadeEnvironment
{
private:
    RegistryFunctionGlobal* _registryFunctionGlobal;

    llvm::BumpPtrAllocator _arena;

    std::unique_ptr<NodeDataRegistry> _nodeDataRegistry;
    std::unique_ptr<IdGenerator> _idGenerator;

    std::unique_ptr<LlvmBackend> _backend;
    std::unique_ptr<RegistryInstruction> _registryInstruction;
    std::unique_ptr<RegistryVariable> _registryVariable;
    std::unique_ptr<RegistryFunctionLocal> _registryFunctionLocal;
    std::unique_ptr<RegistryType> _registryType;
    std::unique_ptr<ReturnContextCompilation> _returnContextCompilation;
    std::unique_ptr<RegistryArgument> _registryArgument;
    std::unique_ptr<RegistryClass> _registryClass;
    std::unique_ptr<ContextGenCode> _context;

    RegistryExpression* _registryExpression;
    BuilderTreeInstruction* _builderTreeInstruction;
    BuilderFloatEquation* _builderEquation;
    TypeParser* _parserType;
    ContextParser* _contextParser;
    ContextExpression* _contextExpression;

    void createRegistries();
    void createGenerators();
    void createContext(const std::string& filePath);
    void createContextParser();
    void registerExternalFunctions();
    void registerBaseTypes();
    void registerExpressions();
    void registerInstructions();

public:
    explicit ConfigurationFacadeEnvironment(RegistryFunctionGlobal* registryFunctionGlobal, PRYSMA_MAYBE_UNUSED FileRegistry* registryFile);
    ~ConfigurationFacadeEnvironment();

    ConfigurationFacadeEnvironment(const ConfigurationFacadeEnvironment&) = delete;
    auto operator=(const ConfigurationFacadeEnvironment&) -> ConfigurationFacadeEnvironment& = delete;
    ConfigurationFacadeEnvironment(ConfigurationFacadeEnvironment&&) = delete;
    auto operator=(ConfigurationFacadeEnvironment&&) -> ConfigurationFacadeEnvironment& = delete;

    /// Initializes the entire compilation environment in a single step
    void initialize(const std::string& filePath);
    
    PRYSMA_NODISCARD auto getContext() const -> ContextGenCode*;
    auto getArena() -> llvm::BumpPtrAllocator&;
    PRYSMA_NODISCARD auto getBuilderTreeInstruction() const -> BuilderTreeInstruction*;
    PRYSMA_NODISCARD auto getBuilderEquation() const -> BuilderFloatEquation*;
};

#endif /* D8FAA486_F5BA_43FB_BFFC_AB9990B46458 */
