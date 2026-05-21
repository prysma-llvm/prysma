//===-- visitor_assignment_variable.cpp -------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/llvm/gestion_variable.h"
#include <llvm-18/llvm/IR/Value.h>

void GeneralVisitorGenCode::visiter(NodeAssignmentVariable* nodeAssignmentVariable)
{
    auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(nodeAssignmentVariable);

    llvm::Value* expressionResult = evaluateExpression(nodeData.getExpression()).getAddress();

    VariableAddressor addressor(_contextGenCode);
    Symbol symbol = addressor.getAddress(nodeData.getToken().value);
    
    llvm::Value* existingVariable = symbol.getAddress();
    
    llvm::Type* variableTypeLLVM = nullptr;
    if (symbol.getType() != nullptr) {
        variableTypeLLVM = symbol.getType()->generateLLVMType(_contextGenCode->getBackend()->getContext());
    }
    
    VariableAssigner assigner(_contextGenCode);
    assigner.assign(existingVariable, expressionResult, variableTypeLLVM);
    
    // Reset the value to avoid problems
    _contextGenCode->setTemporaryValue(Symbol(nullptr, _contextGenCode->getTemporaryValue().getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
}

