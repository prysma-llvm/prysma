//===-- registry_class.h ----------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef C2ADEE91_A0DA_4404_8AF5_5B1105A499EC
#define C2ADEE91_A0DA_4404_8AF5_5B1105A499EC

#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/ast/registry/registry_function.h"
#include "compiler/ast/registry/registry_generic.h"
#include <llvm-18/llvm/ADT/StringRef.h>
#include <llvm-18/llvm/IR/DerivedTypes.h>
#include <llvm-18/llvm/IR/GlobalVariable.h>
#include <string>
#include <map>

class INode;

struct Class
{
private:
    RegistryFunctionLocal* registryFunctionLocal;
    RegistryVariable* registryVariable;

    llvm::GlobalVariable* vtable;
    
    // To automatically compute the object size with llvm 
    llvm::StructType* structType;
    INode* parentInheritance;
    
    // Mapping of member indices for Pass 3
    std::map<llvm::StringRef, unsigned int> memberIndices;
    std::map<std::string, INode*> memberInitializers;
    
    // Mapping of method indices in the vtable
    std::map<std::string, unsigned int> methodIndices;

public:
    // Builder to initialize members
    Class(
        RegistryFunctionLocal* p_registryFunctionLocal,
        RegistryVariable* p_registryVariable,
        llvm::GlobalVariable* p_vtable,
        llvm::StructType* p_structType,
        INode* p_parentInheritance
    )
        : registryFunctionLocal(p_registryFunctionLocal),
          registryVariable(p_registryVariable),
          vtable(p_vtable),
          structType(p_structType),
          parentInheritance(p_parentInheritance)
    {}

    Class(const Class&) = delete;
    auto operator=(const Class&) -> Class& = delete;
    Class(Class&&) = delete;
    auto operator=(Class&&) -> Class& = delete;

    ~Class() {
        delete registryFunctionLocal;
        delete registryVariable;
    }

    Class() : registryFunctionLocal(nullptr), registryVariable(nullptr), vtable(nullptr), structType(nullptr), parentInheritance(nullptr) {}

    void setStructType(llvm::StructType* type) { structType = type; }
    void setParentInheritance(INode* node) { parentInheritance = node; }
    void setRegistryVariable(RegistryVariable* reg) { registryVariable = reg; }
    void setRegistryFunctionLocal(RegistryFunctionLocal* reg) { registryFunctionLocal = reg; }
    void setVTable(llvm::GlobalVariable* vtablePtr) { vtable = vtablePtr; }

    // Getters
    PRYSMA_NODISCARD auto getRegistryFunctionLocal() const -> RegistryFunctionLocal* { return registryFunctionLocal; }
    PRYSMA_NODISCARD auto getRegistryVariable() const -> RegistryVariable* { return registryVariable; }
    PRYSMA_NODISCARD auto getVTable() const -> llvm::GlobalVariable* { return vtable; }
    PRYSMA_NODISCARD auto getStructType() const -> llvm::StructType* { return structType; }
    PRYSMA_NODISCARD auto getParentInheritance() const -> INode* { return parentInheritance; }
    PRYSMA_NODISCARD auto getMemberIndices() -> std::map<llvm::StringRef, unsigned int>& { return memberIndices; }
    PRYSMA_NODISCARD auto getMemberInitializers() -> std::map<std::string, INode*>& { return memberInitializers; }
    PRYSMA_NODISCARD auto getMethodIndices() -> std::map<std::string, unsigned int>& { return methodIndices; }
    
    PRYSMA_NODISCARD auto getMethodIndex(const std::string& methodName) const -> int {
        auto iterator = methodIndices.find(methodName);
        if (iterator != methodIndices.end()) {
            return static_cast<int>(iterator->second);
        }
        return -1;
    }
};
// TODO: support multi-thread mutex for classes. Otherwise, multi-file is not possible.

#include <memory>

class RegistryClass : public RegistryGeneric<std::string, std::unique_ptr<Class>>
{
public:
    RegistryClass() = default;
    RegistryClass(const RegistryClass&) = delete;
    auto operator=(const RegistryClass&) -> RegistryClass& = delete;
    RegistryClass(RegistryClass&&) = delete;
    auto operator=(RegistryClass&&) -> RegistryClass& = delete;

    ~RegistryClass() override = default;
};


#endif /* C2ADEE91_A0DA_4404_8AF5_5B1105A499EC */
