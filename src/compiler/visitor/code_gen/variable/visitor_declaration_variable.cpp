//===-- visitor_declaration_variable.cpp ------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/ast/registry/types/i_type.h"
#include "compiler/lexer/lexer.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/llvm/gestion_variable.h"
#include "compiler/ast/registry/types/type_array.h"
#include <cstddef>
#include <cstdint>
#include <llvm-18/llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>
#include <vector>

#include "compiler/visitor/code_gen/helper/error_helper.h"
#include "compiler/utils/prysma_cast.h"

void GeneralVisitorGenCode::visiter(NodeDeclarationVariable* nodeDeclarationVariable) 
{
    VariableAllocator allocator(_contextGenCode);

    auto& nodeDeclarationData = _contextGenCode->getNodeDataRegistry()->get(nodeDeclarationVariable);

    INode* expression = nodeDeclarationData.getExpression();

    // Check if the expression is an array initialization
    auto* arrayInit = prysma::dyn_cast<NodeArrayInitialization>(expression);

    llvm::AllocaInst* createdAlloca = nullptr; 
    
    if (arrayInit != nullptr) {
        // Determine the LLVM type of the array
        llvm::Type* variableType = nullptr;
        llvm::Type* elementType = nullptr;

        auto& nodeArrData = _contextGenCode->getNodeDataRegistry()->get(arrayInit);
        auto nodeElements = nodeArrData.getElements();
        
        IType* typeDecl = nodeDeclarationData.getType();
        auto* typeArrayDecl = prysma::dyn_cast<TypeArray>(typeDecl);
        
        if (typeArrayDecl != nullptr && typeArrayDecl->getSize() == nullptr) {

            std::size_t realSize = nodeElements.size();

            elementType = typeArrayDecl->getChildType()->generateLLVMType(_contextGenCode->getBackend()->getContext());
            variableType = llvm::ArrayType::get(elementType, realSize);
        } else {
            variableType = nodeDeclarationData.getType()->generateLLVMType(_contextGenCode->getBackend()->getContext());
            if (variableType == nullptr) {
                ErrorHelper::compilationError("Unable to determine array size");
            }
            auto* typeArrayLLVM = llvm::dyn_cast<llvm::ArrayType>(variableType);
            if (typeArrayLLVM == nullptr) {
                ErrorHelper::compilationError("Variable type is not an array for array initialization");
            }
            elementType = typeArrayLLVM->getElementType();
        }
        
        // Allocate and register the array
        llvm::AllocaInst* allocaInstArray = allocator.allocate(variableType, nodeDeclarationData.getName().value);
        createdAlloca = allocaInstArray;

        // Initialize each element of the array
        auto* typeArrayLLVM = llvm::dyn_cast<llvm::ArrayType>(variableType);
        for (std::size_t i = 0; i < nodeElements.size(); ++i) {
            std::vector<llvm::Value*> indices = {
                _contextGenCode->getBackend()->getBuilder().getInt32(0),
                _contextGenCode->getBackend()->getBuilder().getInt32(static_cast<uint32_t>(i))
            }; 
            llvm::Value* ptrElement = _contextGenCode->getBackend()->getBuilder().CreateGEP(typeArrayLLVM, allocaInstArray, indices, "ptr_element");
            
            INode* element = nodeElements[i]; 
            Symbol symbolElement = evaluateExpression(element);
            llvm::Value* castedValue = _contextGenCode->getBackend()->createAutoCast(symbolElement.getAddress(), elementType);
            _contextGenCode->getBackend()->getBuilder().CreateStore(castedValue, ptrElement);
        }
    } else {
        // Simple (non-array) variable
        llvm::Type* variableType = nodeDeclarationData.getType()->generateLLVMType(_contextGenCode->getBackend()->getContext());
        llvm::AllocaInst* allocaInst = allocator.allocate(variableType, nodeDeclarationData.getName().value);
        createdAlloca = allocaInst;
        
        llvm::Value* calculatedValue = evaluateExpression(expression).getAddress();
        if (calculatedValue == nullptr) {
            return;
        }

        llvm::Value* castedValue = _contextGenCode->getBackend()->createAutoCast(calculatedValue, variableType);
        allocator.store(castedValue, allocaInst);
    }
    if (createdAlloca != nullptr) {
        Token token;
        token.value = nodeDeclarationData.getName().value;
  
        // Create the symbol with the pointed type if it's a pointer
        llvm::Type* variableType = nodeDeclarationData.getType()->generateLLVMType(_contextGenCode->getBackend()->getContext());
        
        // If we have an array with dynamic size (nullptr), use the real type we calculated
        if (variableType == nullptr) {
            IType* typeDecl = nodeDeclarationData.getType();
            auto* typeArrayDecl = prysma::dyn_cast<TypeArray>(typeDecl);
            if (typeArrayDecl != nullptr && arrayInit != nullptr) {
                auto& nodeArrData = _contextGenCode->getNodeDataRegistry()->get(arrayInit);
                auto nodeElements = nodeArrData.getElements();

                // Recalculate from the initialization
                std::size_t realSize = nodeElements.size();
                llvm::Type* elementType = typeArrayDecl->getChildType()->generateLLVMType(_contextGenCode->getBackend()->getContext());
                variableType = llvm::ArrayType::get(elementType, realSize);
            } else {
                ErrorHelper::compilationError("Unable to determine variable type");
            }
        }
        
        llvm::Type* elementPointerType = nullptr;
        
        if (variableType->isPointerTy()) {
            // For a pointer, get the pointed type memorized during expression evaluation (new)
            elementPointerType = _contextGenCode->getTemporaryValue().getPointedElementType();
        }
        
        _contextGenCode->getRegistryVariable()->registerVariable(
            token, 
            Symbol(createdAlloca, nodeDeclarationData.getType(), elementPointerType)
        );
    }
}
