//===-- visitor_node_negation.cpp -------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/ast/registry/types/type_simple.h"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include "compiler/visitor/code_gen/helper/error_helper.h"

void GeneralVisitorGenCode::visiter(NodeNegation* node)
{
    auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(node);

    // Safety check
    if (node == nullptr || nodeData.getOperand() == nullptr) {
        ErrorHelper::compilationError("NodeNegation or operand invalid");
    }
    
    // Evaluate the operand (must be boolean)
    nodeData.getOperand()->accept(this);
    llvm::Value* operandVal = _contextGenCode->getTemporaryValue().getAddress();

    auto& builder = _contextGenCode->getBackend()->getBuilder();
    llvm::LLVMContext& context = _contextGenCode->getBackend()->getContext();
    
    // Ensure the operand is of type i1 (bool)
    if (!operandVal->getType()->isIntegerTy(1)) {
        operandVal = builder.CreateTrunc(operandVal, llvm::Type::getInt1Ty(context), "tobool");
    }
    
    // Create logical negation with NOT
    llvm::Value* result = builder.CreateNot(operandVal, "not");

    _contextGenCode->setTemporaryValue(Symbol(result, _contextGenCode->getTemporaryValue().getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
    _contextGenCode->setTemporaryValue(Symbol(_contextGenCode->getTemporaryValue().getAddress(), new (_contextGenCode->getArena()->Allocate<TypeSimple>()) TypeSimple(llvm::Type::getInt1Ty(context)), _contextGenCode->getTemporaryValue().getPointedElementType()));
}
