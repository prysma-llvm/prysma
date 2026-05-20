//===-- visitor_return.cpp --------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/ast/registry/types/i_type.h"
#include <llvm-18/llvm/IR/Value.h>

void GeneralVisitorGenCode::visiter(NodeReturn* nodeReturn)
{
    auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(nodeReturn);

    nodeData.getReturnValue()->accept(this);

    llvm::Value* evaluatedValue = _contextGenCode->getTemporaryValue().getAddress();
    IType* returnTypeObj = _contextGenCode->getReturnContextCompilation()->getContext();
    llvm::Type* returnTypeLLVM = returnTypeObj->generateLLVMType(_contextGenCode->getBackend()->getContext());
    llvm::Value* returnValue = _contextGenCode->getBackend()->createAutoCast(evaluatedValue, returnTypeLLVM);

    _contextGenCode->getBackend()->getBuilder().CreateRet(returnValue);
    _contextGenCode->setTemporaryValue(Symbol(returnValue, _contextGenCode->getTemporaryValue().getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
}