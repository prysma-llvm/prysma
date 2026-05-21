//===-- visitor_access_attribute.cpp ----------------------------*- C++ -*-===//
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
#include "compiler/ast/registry/registry_class.h"
#include "compiler/visitor/code_gen/helper/error_helper.h"
#include <llvm-18/llvm/ADT/StringRef.h>
#include <llvm/IR/Instructions.h>
#include <string>

void GeneralVisitorGenCode::visiter(NodeAccesAttribute* nodeAccessAttribute)
{
    auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(nodeAccessAttribute);

    llvm::StringRef objectName = nodeData.getObjectName().value;
    llvm::StringRef attributeName = nodeData.getAttributeName().value;

    VariableLoader loader(_contextGenCode);
    Symbol objectSymbol = loader.load(objectName);
    llvm::Value* object = objectSymbol.getAddress();

    std::string className = getClassNameFromSymbol(objectSymbol);

    if (className.empty()) {
        ErrorHelper::compilationError("Unable to determine the class of object '" + objectName.str() + "'");
    }

    auto* classInfo = _contextGenCode->getRegistryClass()->get(className).get();
    classInfo = ErrorHelper::verifyNotNull(classInfo, "Class '" + className + "' not found in the registry");

    if (!classInfo->getRegistryVariable()->variableExists(attributeName.str())) {
        ErrorHelper::compilationError("Attribute '" + attributeName.str() + "' does not exist in class '" + className + "'");
    }

    auto iterator = classInfo->getMemberIndices().find(llvm::StringRef(attributeName).str());
    if (iterator == classInfo->getMemberIndices().end()) {
        ErrorHelper::compilationError(
            ("Attribute '" + attributeName + "' has no index in " + className).str()
        );
    }
    
    unsigned int index = iterator->second;

    auto& builder = _contextGenCode->getBackend()->getBuilder();

    Symbol varSymbol = classInfo->getRegistryVariable()->getVariable(nodeData.getAttributeName());
    
    llvm::Type* structType = classInfo->getStructType();

    llvm::Value* attributePtr = builder.CreateStructGEP(structType, object, index, objectName + "_" + attributeName + "_ptr");

    llvm::Value* attributeValue = builder.CreateLoad(
        varSymbol.getType()->generateLLVMType(_contextGenCode->getBackend()->getContext()), 
        attributePtr, 
        objectName + "_" + attributeName
    );

    _contextGenCode->setTemporaryValue(Symbol(attributeValue, varSymbol.getType(), nullptr));
}
