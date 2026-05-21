//===-- visitor_node_literal.cpp --------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/ast/registry/types/type_simple.h"
#include "compiler/llvm/gestion_variable.h"
#include "compiler/visitor/code_gen/helper/error_helper.h"
#include <cstdint>
#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/FormatVariadic.h>
#include <string>

void GeneralVisitorGenCode::visiter(NodeLiteral* nodeLiteral)
{
    llvm::LLVMContext& context = _contextGenCode->getBackend()->getContext();

    auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(nodeLiteral);

    Token token = nodeData.getToken();

    if (token.type == TOKEN_IDENTIFIER) {
        VariableLoader loader(_contextGenCode);
        _contextGenCode->setTemporaryValue(loader.load(token.value));
        return; 
    }

    llvm::Value* llvmValue = nullptr;
    llvm::Type* llvmType = nullptr;

    if (token.type == TOKEN_TRUE || token.type == TOKEN_FALSE) {
        bool value = (token.type == TOKEN_TRUE);
        llvmType = llvm::Type::getInt1Ty(context);
        llvmValue = llvm::ConstantInt::get(llvmType, value ? 1 : 0);
    }
    else if (token.type == TOKEN_LIT_BOOL) {
        llvm::StringRef str_val = token.value; 
        long long value = std::stoll(std::string(str_val));
        llvmType = llvm::Type::getInt1Ty(context);
        llvmValue = llvm::ConstantInt::get(llvmType, value != 0 ? 1 : 0);
    }
    else if (token.type == TOKEN_LIT_INT) { 
        llvm::StringRef str_val = token.value; 
        long long value = std::stoll(std::string(str_val)); 
        // By default, if it exceeds 32 bits, we could allocate it as 64-bit,
        // but for now we allocate at least 64 bits or follow the original code with 'stoll' instead of 'stoi'
        // to avoid crashing in C++. The type will be adapted later during the operator (createAutoCast handles this).
        llvmType = llvm::Type::getInt64Ty(context);
        llvmValue = llvm::ConstantInt::get(llvmType, static_cast<uint64_t>(value), true);
    }
    else if (token.type == TOKEN_LIT_FLOAT) { 
        float value = std::stof(std::string(token.value)); 
        llvmType = llvm::Type::getFloatTy(context);
        llvmValue = llvm::ConstantFP::get(llvmType, value);
    }
    else {
        ErrorHelper::compilationError(llvm::formatv("Unsupported literal type ({0})", token.value).str());
    }

    _contextGenCode->setTemporaryValue(Symbol(llvmValue, _contextGenCode->getTemporaryValue().getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
    _contextGenCode->setTemporaryValue(Symbol(_contextGenCode->getTemporaryValue().getAddress(), new (_contextGenCode->getArena()->Allocate<TypeSimple>()) TypeSimple(llvmType), _contextGenCode->getTemporaryValue().getPointedElementType()));
}
