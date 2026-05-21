//===-- visitor_operation.cpp -----------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/lexer/token_type.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/ast/registry/types/type_simple.h"
#include <llvm-18/llvm/IR/Value.h>
#include <llvm/IR/Type.h>

#include "compiler/visitor/code_gen/helper/error_helper.h"

void GeneralVisitorGenCode::visiter(NodeOperation* node)
{
    auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(node);

    // Evaluate the left side and SAVE the complete Symbol
    nodeData.getLeft()->accept(this);
    Symbol leftSym = _contextGenCode->getTemporaryValue();

    // Evaluate the right side (the left temporary value is not overwritten)
    nodeData.getRight()->accept(this);
    Symbol rightSym = _contextGenCode->getTemporaryValue();

    llvm::Value* leftVal = leftSym.getAddress();
    llvm::Value* rightVal = rightSym.getAddress();

    auto& builder = _contextGenCode->getBackend()->getBuilder();
    llvm::Value* result = nullptr;
    llvm::Type* resultType = nullptr;

    // Prysma decides the type, not LLVM
    bool isFloating = (leftSym.getType() != nullptr && leftSym.getType()->isFloating()) || 
                      (rightSym.getType() != nullptr && rightSym.getType()->isFloating());

    if (isFloating) {
        llvm::Type* floatTy = llvm::Type::getFloatTy(_contextGenCode->getBackend()->getContext());
        leftVal = _contextGenCode->getBackend()->createAutoCast(leftVal, floatTy);
        rightVal = _contextGenCode->getBackend()->createAutoCast(rightVal, floatTy);
        resultType = floatTy;

        switch (nodeData.getToken().type) {
            case TOKEN_PLUS:  result = builder.CreateFAdd(leftVal, rightVal, "fadd"); break;
            case TOKEN_MINUS: result = builder.CreateFSub(leftVal, rightVal, "fsub"); break;
            case TOKEN_STAR:  result = builder.CreateFMul(leftVal, rightVal, "fmul"); break;
            case TOKEN_SLASH: result = builder.CreateFDiv(leftVal, rightVal, "fdiv"); break;
            case TOKEN_MODULO: result = builder.CreateFRem(leftVal, rightVal, "frem"); break;
            // For float comparisons, returns i1 (bool)
            case TOKEN_LESS:      result = builder.CreateFCmpOLT(leftVal, rightVal, "fcmp_lt"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            case TOKEN_GREATER:   result = builder.CreateFCmpOGT(leftVal, rightVal, "fcmp_gt"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            case TOKEN_LESS_EQUAL: result = builder.CreateFCmpOLE(leftVal, rightVal, "fcmp_le"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            case TOKEN_GREATER_EQUAL: result = builder.CreateFCmpOGE(leftVal, rightVal, "fcmp_ge"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            case TOKEN_EQUAL_EQUAL: result = builder.CreateFCmpOEQ(leftVal, rightVal, "fcmp_eq"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            case TOKEN_NOT_EQUAL: result = builder.CreateFCmpONE(leftVal, rightVal, "fcmp_ne"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            default: break;
        }
    } 
    else {
        llvm::Type* targetIntTy = nullptr;
        if (leftVal->getType()->isIntegerTy() && rightVal->getType()->isIntegerTy()) {
            unsigned leftBits = leftVal->getType()->getIntegerBitWidth();
            unsigned rightBits = rightVal->getType()->getIntegerBitWidth();
            targetIntTy = (leftBits > rightBits) ? leftVal->getType() : rightVal->getType();
        } else {
            targetIntTy = llvm::Type::getInt32Ty(_contextGenCode->getBackend()->getContext());
        }

        leftVal = _contextGenCode->getBackend()->createAutoCast(leftVal, targetIntTy);
        rightVal = _contextGenCode->getBackend()->createAutoCast(rightVal, targetIntTy);
        
        resultType = targetIntTy;

        switch (nodeData.getToken().type) {
            case TOKEN_PLUS:  result = builder.CreateAdd(leftVal, rightVal, "iadd"); break;
            case TOKEN_MINUS: result = builder.CreateSub(leftVal, rightVal, "isub"); break;
            case TOKEN_STAR:  result = builder.CreateMul(leftVal, rightVal, "imul"); break;
            case TOKEN_SLASH: result = builder.CreateSDiv(leftVal, rightVal, "idiv"); break;
            case TOKEN_MODULO: result = builder.CreateSRem(leftVal, rightVal, "irem"); break;
            
            // Integer comparisons (ICmp)
            case TOKEN_LESS:      result = builder.CreateICmpSLT(leftVal, rightVal, "icmp_lt"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            case TOKEN_GREATER:   result = builder.CreateICmpSGT(leftVal, rightVal, "icmp_gt"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            case TOKEN_LESS_EQUAL: result = builder.CreateICmpSLE(leftVal, rightVal, "icmp_le"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            case TOKEN_GREATER_EQUAL: result = builder.CreateICmpSGE(leftVal, rightVal, "icmp_ge"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            case TOKEN_EQUAL_EQUAL: result = builder.CreateICmpEQ(leftVal, rightVal, "icmp_eq"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            case TOKEN_NOT_EQUAL: result = builder.CreateICmpNE(leftVal, rightVal, "icmp_ne"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            case TOKEN_AND:      result = builder.CreateAnd(leftVal, rightVal, "and"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            case TOKEN_OR:       result = builder.CreateOr(leftVal, rightVal, "or"); resultType = llvm::Type::getInt1Ty(builder.getContext()); break;
            default: break;
        }
    }

    result = ErrorHelper::verifyNotNull(result, "Unknown operation");

    _contextGenCode->setTemporaryValue(Symbol(result, _contextGenCode->getTemporaryValue().getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
    _contextGenCode->setTemporaryValue(Symbol(_contextGenCode->getTemporaryValue().getAddress(), new (_contextGenCode->getArena()->Allocate<TypeSimple>()) TypeSimple(resultType), _contextGenCode->getTemporaryValue().getPointedElementType()));
}
