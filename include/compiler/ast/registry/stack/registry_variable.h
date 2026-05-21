//===-- registry_variable.h -------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef F7F44FF6_77D4_456C_A897_1A52149FDE53
#define F7F44FF6_77D4_456C_A897_1A52149FDE53

#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/lexer/lexer.h"
#include "compiler/ast/registry/types/i_type.h"
#include <llvm-18/llvm/ADT/StringMap.h>
#include <llvm-18/llvm/IR/Value.h>
#include <map>
#include <stack>
#include <string>

namespace llvm { class AllocaInst; }

struct Symbol {
private:
    llvm::Value* address;
    IType* type;
    llvm::Type* pointedElementType;

public:
    Symbol() : address(nullptr), type(nullptr), pointedElementType(nullptr) {}

    Symbol(llvm::Value* pAddress, IType* pType) : address(pAddress), type(pType), pointedElementType(nullptr) {}

    Symbol(llvm::Value* pAddress, IType* pType, llvm::Type* pPointedElementType)
        : address(pAddress), type(pType), pointedElementType(pPointedElementType) {}

    PRYSMA_NODISCARD auto getAddress() const -> llvm::Value* { return address; }
    PRYSMA_NODISCARD auto getType() const -> IType* { return type; }
    PRYSMA_NODISCARD auto getPointedElementType() const -> llvm::Type* { return pointedElementType; }
};

class RegistryVariable 
{
private: 
    std::stack<llvm::StringMap<Symbol>>  _variables; 

public:
    RegistryVariable();
    
    ~RegistryVariable();

    RegistryVariable(const RegistryVariable&) = delete;
    auto operator=(const RegistryVariable&) -> RegistryVariable& = delete;
    RegistryVariable(RegistryVariable&&) = delete;
    auto operator=(RegistryVariable&&) -> RegistryVariable& = delete;
    
    auto getVariable(const Token& token) -> Symbol;

    void registerVariable(
        const Token& token,
        Symbol symbol
    );
    
    void push();
    void pop();
    void clearTop();

    auto variableExists(const std::string& name) -> bool;

    auto getGlobalVariables() -> llvm::StringMap<Symbol>& {
        return _variables.top();
    }
};

#endif /* F7F44FF6_77D4_456C_A897_1A52149FDE53 */
