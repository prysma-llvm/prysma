//===-- visitor_call_object.cpp ---------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/ast/registry/registry_function.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/llvm/gestion_variable.h"
#include "compiler/ast/registry/registry_class.h"
#include "compiler/visitor/code_gen/helper/error_helper.h"
#include "compiler/visitor/code_gen/helper/v_table_navigator.h"
#include "compiler/utils/prysma_cast.h"
#include <llvm-18/llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>
#include <stdexcept>
#include <string>
#include <vector>

void GeneralVisitorGenCode::visiter(NodeCallObject* nodeCallObject)
{
    auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(nodeCallObject);

    // Retrieve the object name (e.g., "dog")
    llvm::StringRef objectName = nodeData.getObjectName().value;

    // Retrieve the called method name (e.g., "bark")
    llvm::StringRef methodName = nodeData.getMethodName().value;

    VariableLoader loader(_contextGenCode);
    
    // Load the object variable (which is a pointer)
    Symbol objectSymbol = loader.load(objectName);
    llvm::Value* object = objectSymbol.getAddress();

    std::string className = getClassNameFromSymbol(objectSymbol);

    if (className.empty()) {
        ErrorHelper::compilationError("Class of object '" + std::string(objectName) + "' undetermined");
    }
    // Create the path to the object
    auto* structClassType = llvm::dyn_cast<llvm::StructType>(objectSymbol.getPointedElementType());
    llvm::Value* vptrAddress = _contextGenCode->getBackend()->getBuilder().CreateStructGEP(structClassType, object, 0, "vptr_address");

    // Retrieve the class vtable containing the object's methods (e.g., "Dog")
    // We must build the vtable dynamically from the vptr address in the object, because in case of inheritance the vtable may be different
    // from the static class vtable of the object
    llvm::Value* vtable = _contextGenCode->getBackend()->getBuilder().CreateLoad(_contextGenCode->getBackend()->getBuilder().getPtrTy(), vptrAddress, "vptr");
    // Get the method index in the vtable for indirect call
    int methodIndex = _contextGenCode->getRegistryClass()->get(className)->getMethodIndex(methodName.str());

    auto* classInfo = _contextGenCode->getRegistryClass()->get(className).get();
    classInfo = ErrorHelper::verifyNotNull(classInfo, "Class '" + className + "' not found");

    ErrorHelper::verifyExistence(*classInfo->getRegistryFunctionLocal(), methodName.str(),
        "Method '" + methodName.str() + "' does not exist in '" + className + "'");

    auto& builder = _contextGenCode->getBackend()->getBuilder();
    
    VTableNavigator navigator(&builder);
    
    llvm::Value* methodPointer = navigator.getMethodPointer(vtable, classInfo->getVTable()->getValueType(), methodIndex);
    
    // The first argument to pass to the method is the "this" pointer (the object itself)
    std::vector<llvm::Value*> args;
    args.push_back(object);

    const auto& symbolPtr = classInfo->getRegistryFunctionLocal()->get(methodName.str());
    if (!prysma::isa<SymbolFunctionLocal>(symbolPtr.get())) {
        throw std::runtime_error("Error: SymbolFunctionLocal expected");
    }
    const auto* functionSymbol = prysma::cast<const SymbolFunctionLocal>(symbolPtr.get());
    llvm::FunctionType* functionType = functionSymbol->function->getFunctionType();

    unsigned int paramIndex = 1; // 0 is "this"
    for (INode* argumentChild : nodeData.getChildren()) 
    {
        argumentChild->accept(this);
        llvm::Value* argumentValue = _contextGenCode->getTemporaryValue().getAddress();
        
        argumentValue = ErrorHelper::verifyNotNull(argumentValue, "Object call argument did not generate a value");

        llvm::Value* finalValue = argumentValue;
        if (paramIndex < functionType->getNumParams()) {
            llvm::Type* expectedType = functionType->getParamType(paramIndex);
            finalValue = _contextGenCode->getBackend()->createAutoCast(argumentValue, expectedType);
        }
        args.push_back(finalValue);
        paramIndex++;
    }

    // If the method is the builder then call the static initialization method
    if(methodName == className)
    {
        builder.CreateCall(functionSymbol->function, args);
        return;
    }

    // Use the vtable for an indirect call to the method
    if (functionSymbol->function->getReturnType()->isVoidTy()) { 
         builder.CreateCall(functionType, methodPointer, args);
        _contextGenCode->setTemporaryValue(Symbol(nullptr, _contextGenCode->getTemporaryValue().getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
        _contextGenCode->setTemporaryValue(Symbol(_contextGenCode->getTemporaryValue().getAddress(), nullptr, _contextGenCode->getTemporaryValue().getPointedElementType()));
    } else {
        llvm::Value* result = builder.CreateCall(functionType, methodPointer, args, "object_call_result");
        _contextGenCode->setTemporaryValue(Symbol(result, _contextGenCode->getTemporaryValue().getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
        _contextGenCode->setTemporaryValue(Symbol(_contextGenCode->getTemporaryValue().getAddress(), functionSymbol->returnType, _contextGenCode->getTemporaryValue().getPointedElementType()));
    }
}
