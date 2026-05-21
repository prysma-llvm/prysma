//===-- visitor_reading_array.cpp -------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/ast/registry/types/i_type.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/ast/registry/types/type_simple.h"
#include "compiler/ast/registry/types/type_array.h"
#include "compiler/llvm/gestion_variable.h"
#include "compiler/utils/prysma_cast.h"
#include "compiler/visitor/code_gen/helper/error_helper.h"
#include <cstddef>
#include <llvm-18/llvm/IR/Instructions.h>
#include <llvm-18/llvm/Support/Casting.h>
#include <string>

void GeneralVisitorGenCode::visiter(NodeReadingArray* nodeReadingArray)
{
    auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(nodeReadingArray);

    llvm::StringRef arrayNameStr = nodeData.getName().value;
    Symbol symbol;
    llvm::Value* arrayAddress = nullptr;

    if (arrayNameStr.find('.') != std::string::npos) {
        size_t pos = arrayNameStr.find('.');
        llvm::StringRef objectName = arrayNameStr.substr(0, pos);
        llvm::StringRef attributeName = arrayNameStr.substr(pos + 1);

        VariableLoader loader(_contextGenCode);
        Symbol objectSymbol = loader.load(objectName);
        llvm::Value* object = objectSymbol.getAddress();

        std::string className = getClassNameFromSymbol(objectSymbol);
        auto* classInfo = _contextGenCode->getRegistryClass()->get(className).get();
        auto iterator = classInfo->getMemberIndices().find(attributeName.str());
        unsigned int indexObj = iterator->second;

        Token tokenAttribute; tokenAttribute.value = attributeName; tokenAttribute.type = TOKEN_IDENTIFIER;
        symbol = classInfo->getRegistryVariable()->getVariable(tokenAttribute);
        
        llvm::Type* structType = classInfo->getStructType();
        arrayAddress = _contextGenCode->getBackend()->getBuilder().CreateStructGEP(structType, object, indexObj, objectName + "_" + attributeName + "_ptr");
    } else {
        VariableAddressor addressor(_contextGenCode);
        symbol = addressor.getAddress(arrayNameStr);
        arrayAddress = symbol.getAddress();
    }

    llvm::Type* arrayType = nullptr;
    if (symbol.getType() != nullptr) {
        arrayType = symbol.getType()->generateLLVMType(_contextGenCode->getBackend()->getContext());
    } else {
        auto* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(arrayAddress);
        allocaInst = ErrorHelper::verifyNotNull(allocaInst, "The array address is not an allocation instruction");
        arrayType = allocaInst->getAllocatedType();
    }
    
    // Compute the index from the equation 
    nodeData.getIndexEquation()->accept(this);
    llvm::Value* indexValue = _contextGenCode->getTemporaryValue().getAddress();
    
    llvm::Value* elementAddress = nullptr;
    llvm::Type* elementType = nullptr;

    if (arrayType->isArrayTy()) {
        // compute the offset with GEP 
        elementAddress = _contextGenCode->getBackend()->getBuilder().CreateGEP(arrayType, arrayAddress, 
            { 
                _contextGenCode->getBackend()->getBuilder().getInt32(0), 
                indexValue 
            }, nodeData.getName().value);
        elementType = arrayType->getArrayElementType();
    } else {
        // It's a pointer to an array (reference), must load the pointer
        llvm::Value* loadedPtr = _contextGenCode->getBackend()->getBuilder().CreateLoad(arrayType, arrayAddress);
        
        IType* astType = symbol.getType();
        auto* declaredArrayType = prysma::dyn_cast<TypeArray>(astType);
        if (declaredArrayType != nullptr) {
            elementType = declaredArrayType->getChildType()->generateLLVMType(_contextGenCode->getBackend()->getContext());
        } else {
            elementType = symbol.getPointedElementType();
        }
        
        elementAddress = _contextGenCode->getBackend()->getBuilder().CreateGEP(
            elementType, 
            loadedPtr, 
            indexValue,
            nodeData.getName().value
        );
    }
        
    // Load the value from the element address
    llvm::Value* elementValue = _contextGenCode->getBackend()->getBuilder().CreateLoad(elementType, elementAddress, nodeData.getName().value);

    _contextGenCode->setTemporaryValue(Symbol(elementValue, _contextGenCode->getTemporaryValue().getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
    _contextGenCode->setTemporaryValue(Symbol(_contextGenCode->getTemporaryValue().getAddress(), new (_contextGenCode->getArena()->Allocate<TypeSimple>()) TypeSimple(elementType), _contextGenCode->getTemporaryValue().getPointedElementType()));
}

