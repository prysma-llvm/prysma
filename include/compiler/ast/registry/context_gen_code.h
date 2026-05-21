//===-- context_gen_code.h --------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef C2537ED8_1CCF_4242_BDB0_B5ED5F2AD08F
#define C2537ED8_1CCF_4242_BDB0_B5ED5F2AD08F

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include "compiler/ast/registry/data/id_generator.hpp"
#include "compiler/ast/registry/data/node_data_registry.hpp"
#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/ast/registry/registry_type.h"
#include "registry_instruction.h"
#include "registry_instruction.h"
#include "registry_function.h"
#include "registry_type.h"
#include "registry_argument.h"
#include "registry_class.h"
#include "stack/return_context_compilation.h"
#include "../../llvm/llvm_backend.h"
#include "llvm/Support/Allocator.h"

struct ContextGenCode
{
private: // TODO: adapter le naming scheme vers _membre
    RegistryType* registryType;
    Symbol temporaryValue;
    LlvmBackend* backend;
    NodeDataRegistry* nodeDataRegistry;
    IdGenerator* idGenerator;
    RegistryInstruction* registryInstruction;
    RegistryVariable* registryVariable;
    RegistryFunctionGlobal* registryFunctionGlobal;
    RegistryFunctionLocal* registryFunctionLocal;
    ReturnContextCompilation* returnContextCompilation;
    RegistryArgument* registryArgument;
    RegistryClass* registryClass;
    llvm::BumpPtrAllocator* arena;
    std::string currentFilePath;
    std::string currentClassName;

public:
    ContextGenCode(
        RegistryType* p_registryType,
        LlvmBackend* p_backend,
        NodeDataRegistry* p_nodeDataRegistry,
        IdGenerator* p_idGenerator,
        RegistryInstruction* p_registryInstruction,
        RegistryVariable* p_registryVariable,
        RegistryFunctionGlobal* p_registryFunctionGlobal,
        RegistryFunctionLocal* p_registryFunctionLocal,
        ReturnContextCompilation* p_returnContextCompilation,
        RegistryArgument* p_registryArgument,
        RegistryClass* p_registryClass,
        Symbol p_temporaryValue,
        llvm::BumpPtrAllocator* p_arena,
        std::string p_currentFilePath
    ) 
    {
        try {
            if (p_currentFilePath.empty()) {
                throw std::invalid_argument("The current file path cannot be empty");
            }
            if (p_registryType == nullptr)
            {
                throw std::invalid_argument("The file registry cannot be null");
            }
            if (p_backend == nullptr) {
                throw std::invalid_argument("The LLVM backend cannot be null");
            }
            if (p_nodeDataRegistry == nullptr) {
                throw std::invalid_argument("The node data registry cannot be null");
            }
            if (p_idGenerator == nullptr) {
                throw std::invalid_argument("The id generator cannot be null");
            }
            if (p_registryInstruction == nullptr) {
                throw std::invalid_argument("The instruction registry cannot be null");
            }
            if (p_registryVariable == nullptr) {
                throw std::invalid_argument("The variable registry cannot be null");
            }
            if (p_registryFunctionGlobal == nullptr) {
                throw std::invalid_argument("The global function registry cannot be null");
            }
            if (p_registryFunctionLocal == nullptr) {
                throw std::invalid_argument("The local function registry cannot be null");
            }
            if (p_registryType == nullptr) {
                throw std::invalid_argument("The type registry cannot be null");
            }
            if (p_returnContextCompilation == nullptr) {
                throw std::invalid_argument("The return context compilation cannot be null");
            }
            if (p_registryArgument == nullptr) {
                throw std::invalid_argument("The argument registry cannot be null");
            }
            if (p_arena == nullptr) {
                throw std::invalid_argument("The arena cannot be null");
            }
            if (p_registryClass == nullptr) {
                throw std::invalid_argument("The class registry cannot be null");
            }
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error during code generation context creation: " << e.what() << std::endl;
            throw;
        }
        this->registryType = p_registryType;
        this->currentFilePath = std::move(p_currentFilePath);
        this->backend = p_backend;
        this->nodeDataRegistry = p_nodeDataRegistry;
        this->idGenerator = p_idGenerator;
        this->registryInstruction = p_registryInstruction;
        this->registryVariable = p_registryVariable;
        this->registryFunctionGlobal = p_registryFunctionGlobal;
        this->registryFunctionLocal = p_registryFunctionLocal;
        this->registryType = p_registryType;
        this->returnContextCompilation = p_returnContextCompilation;
        this->registryArgument = p_registryArgument;
        this->registryClass = p_registryClass;
        this->temporaryValue = p_temporaryValue;
        this->arena = p_arena;
    }

    
    void setTemporaryValue(Symbol p_temporaryValue) { temporaryValue = p_temporaryValue; }
    void setCurrentClassName(std::string p_currentClassName) { currentClassName = std::move(p_currentClassName); }

    // Getters
    PRYSMA_NODISCARD auto getRegistryType() const -> RegistryType* { return registryType; }
    PRYSMA_NODISCARD auto getTemporaryValue() const -> Symbol { return temporaryValue; }
    PRYSMA_NODISCARD auto getBackend() const -> LlvmBackend* { return backend; }
    PRYSMA_NODISCARD auto getNodeDataRegistry() const -> NodeDataRegistry* { return nodeDataRegistry; }
    PRYSMA_NODISCARD auto getIdGenerator() const -> IdGenerator* { return idGenerator; }
    PRYSMA_NODISCARD auto getRegistryInstruction() const -> RegistryInstruction* { return registryInstruction; }
    PRYSMA_NODISCARD auto getRegistryVariable() const -> RegistryVariable* { return registryVariable; }
    PRYSMA_NODISCARD auto getRegistryFunctionGlobal() const -> RegistryFunctionGlobal* { return registryFunctionGlobal; }
    PRYSMA_NODISCARD auto getRegistryFunctionLocal() const -> RegistryFunctionLocal* { return registryFunctionLocal; }
    PRYSMA_NODISCARD auto getReturnContextCompilation() const -> ReturnContextCompilation* { return returnContextCompilation; }
    PRYSMA_NODISCARD auto getRegistryArgument() const -> RegistryArgument* { return registryArgument; }
    PRYSMA_NODISCARD auto getRegistryClass() const -> RegistryClass* { return registryClass; }
    PRYSMA_NODISCARD auto getArena() const -> llvm::BumpPtrAllocator* { return arena; }
    PRYSMA_NODISCARD auto getCurrentFilePath() const -> const std::string& { return currentFilePath; }
    PRYSMA_NODISCARD auto getCurrentClassName() const -> const std::string& { return currentClassName; }
};

#endif /* C2537ED8_1CCF_4242_BDB0_B5ED5F2AD08F */
