//===-- visitor_delete.cpp --------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/lexer/lexer.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/visitor/code_gen/helper/error_helper.h"
#include <llvm-18/llvm/IR/Value.h>
#include <llvm/Support/FormatVariadic.h>

void GeneralVisitorGenCode::visiter(NodeDelete* nodeDelete)
{
    auto& module = _contextGenCode->getBackend()->getModule();
    auto& builder = _contextGenCode->getBackend()->getBuilder();

    auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(nodeDelete);

    // Retrieve the token of the variable to delete
    const Token& variableToken = nodeData.getName();

    // Search for the variable in the variable registry to determine if it exists
    Symbol symbol = _contextGenCode->getRegistryVariable()->getVariable(variableToken);

    ErrorHelper::verifyNotNull(symbol.getAddress(), llvm::formatv("Variable '{0}' not declared", variableToken.value).str());
    
    llvm::Value* memoryAddress = symbol.getAddress();
    llvm::Type* dataType = memoryAddress->getType();

    if (!dataType->isPointerTy()) {
        ErrorHelper::compilationError(llvm::formatv("Variable '{0}' is not a pointer (delete requires a pointer)", variableToken.value).str());
    }

    llvm::Value* addressToFree = builder.CreateLoad(
        dataType,
        memoryAddress,
        llvm::formatv("pointer_{0}", variableToken.value).str()
    );

    // Retrieve the prysma_free function from the module
    llvm::Function* freeFunc = module.getFunction("prysma_free");

    freeFunc = ErrorHelper::verifyNotNull(freeFunc, "Function prysma_free not declared");

    // Call prysma_free with the address to free
    builder.CreateCall(freeFunc, {addressToFree});

    // Reset the temporary value
    _contextGenCode->setTemporaryValue(Symbol(nullptr, _contextGenCode->getTemporaryValue().getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
}
