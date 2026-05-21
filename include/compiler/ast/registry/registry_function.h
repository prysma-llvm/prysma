//===-- registry_function.h -------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef F2141F07_2C85_4ADB_9BC9_A909EBD34394
#define F2141F07_2C85_4ADB_9BC9_A909EBD34394

#include "compiler/macros/prysma_nodiscard.h"
#include "registry_generic.h"
#include <llvm-18/llvm/ADT/StringRef.h>
#include <llvm-18/llvm/IR/Function.h>
#include <mutex>
#include <string>
#include <memory>

class IType;
class NodeDeclarationFunction;

class IFunctionSymbolRegistry
{
    public:
        enum class SymbolType { Global, Local };
        IFunctionSymbolRegistry() = default;
        IFunctionSymbolRegistry(const IFunctionSymbolRegistry&) = delete;
        auto operator=(const IFunctionSymbolRegistry&) -> IFunctionSymbolRegistry& = delete;
        IFunctionSymbolRegistry(IFunctionSymbolRegistry&&) = delete;
        auto operator=(IFunctionSymbolRegistry&&) -> IFunctionSymbolRegistry& = delete;
        virtual ~IFunctionSymbolRegistry() = default;
        PRYSMA_NODISCARD virtual SymbolType getType() const = 0;
};

// Acts as a struct to store global functions
// Public members
// No llvm::Function* here, thread safe for the global registry
class SymbolFunctionGlobal : public IFunctionSymbolRegistry {
public: 
    IType* returnType = nullptr;
    NodeDeclarationFunction* node = nullptr;
    
    PRYSMA_NODISCARD auto getType() const -> SymbolType override { return SymbolType::Global; }
    PRYSMA_NODISCARD static auto classof(const IFunctionSymbolRegistry* s) -> bool { 
        return s->getType() == SymbolType::Global; 
    }
};

// Local registry with llvm::Function* for code generation in a thread
class SymbolFunctionLocal : public IFunctionSymbolRegistry {
public:
    llvm::Function* function = nullptr;
    IType* returnType = nullptr;
    NodeDeclarationFunction* node = nullptr;

    PRYSMA_NODISCARD auto getType() const -> SymbolType override { return SymbolType::Local; }
    PRYSMA_NODISCARD static auto classof(const IFunctionSymbolRegistry* s) -> bool { 
        return s->getType() == SymbolType::Local; 
    }
};

class RegistryFunctionGlobal : public RegistryGeneric<std::string, std::unique_ptr<IFunctionSymbolRegistry>, std::mutex>
{
public:
    RegistryFunctionGlobal() = default;
    RegistryFunctionGlobal(const RegistryFunctionGlobal&) = delete;
    auto operator=(const RegistryFunctionGlobal&) -> RegistryFunctionGlobal& = delete;
    RegistryFunctionGlobal(RegistryFunctionGlobal&&) = delete;
    auto operator=(RegistryFunctionGlobal&&) -> RegistryFunctionGlobal& = delete;
    ~RegistryFunctionGlobal() override = default;
};

class RegistryFunctionLocal : public RegistryGeneric<llvm::StringRef, std::unique_ptr<IFunctionSymbolRegistry>>
{
public:
    RegistryFunctionLocal() = default;
    RegistryFunctionLocal(const RegistryFunctionLocal&) = delete;
    auto operator=(const RegistryFunctionLocal&) -> RegistryFunctionLocal& = delete;
    RegistryFunctionLocal(RegistryFunctionLocal&&) = delete;
    auto operator=(RegistryFunctionLocal&&) -> RegistryFunctionLocal& = delete;
    ~RegistryFunctionLocal() override = default;
};

#endif /* F2141F07_2C85_4ADB_9BC9_A909EBD34394 */
