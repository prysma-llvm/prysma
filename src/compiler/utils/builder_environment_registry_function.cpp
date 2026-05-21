//===-- builder_environment_registry_function.cpp ---------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/utils/builder_environment_registry_function.h"
#include "compiler/ast/registry/context_gen_code.h"
#include "compiler/ast/registry/registry_function.h"
#include "compiler/ast/registry/registry_class.h"
#include "compiler/utils/prysma_cast.h"
#include "compiler/visitor/extractors/members_extractor_class.h"
#include <llvm-18/llvm/IR/Function.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

BuilderEnvironmentRegistryFunction::BuilderEnvironmentRegistryFunction(ContextGenCode* contextGenCode)
    : _contextGenCode(contextGenCode) {}

BuilderEnvironmentRegistryFunction::~BuilderEnvironmentRegistryFunction() = default;

void BuilderEnvironmentRegistryFunction::buildVTable(Class* classInfo, const std::string& className, const std::vector<NodeDeclarationFunction*>& parentMethodList)
{
    // Build the vtable by aligning the parent's methods at the same positions
    std::vector<llvm::Constant*> vtableElements;
    
    // Add the parent's function pointers to the vtable, in the same order
    for (NodeDeclarationFunction* parentMethodDecl : parentMethodList) {
        auto& nodeParentMethodDeclData = _contextGenCode->getNodeDataRegistry()->get(parentMethodDecl);

        llvm::StringRef parentMethodName = nodeParentMethodDeclData.getName().value;
        std::string parentMethodNameStr = parentMethodName.str();
        
        // Search for the corresponding method in the current class
        llvm::Function* functionImpl = nullptr;
        if (classInfo->getRegistryFunctionLocal()->exists(parentMethodNameStr)) {
            const auto& symbol = classInfo->getRegistryFunctionLocal()->get(parentMethodNameStr);
            if (!prysma::isa<SymbolFunctionLocal>(symbol.get())) {
                throw std::runtime_error("Error: Expected SymbolFunctionLocal");
            }
            auto* localSymbol = prysma::cast<SymbolFunctionLocal>(symbol.get());
            functionImpl = localSymbol->function;
        }
        
        // If found, add the function pointer to the vtable
        if (functionImpl != nullptr) {
            vtableElements.push_back(llvm::ConstantExpr::getBitCast(
                functionImpl,
                llvm::PointerType::get(llvm::Type::getInt8Ty(_contextGenCode->getBackend()->getContext()), 0)
            ));
            classInfo->getMethodIndices()[parentMethodName.str()] = static_cast<unsigned int>(vtableElements.size() - 1);
        }
    }
    
    // Add additional methods from the current class that are not in the parent
    auto methodKeys = classInfo->getRegistryFunctionLocal()->getKeys();
    for (const auto& key : methodKeys) {
        llvm::StringRef methodName = key;
        bool isInParent = false;
        
        // Check if this method is in the parent
        for (NodeDeclarationFunction* parentMethodDecl : parentMethodList) {
            auto& nodeParentMethodDeclData = _contextGenCode->getNodeDataRegistry()->get(parentMethodDecl);

            if (nodeParentMethodDeclData.getName().value == methodName) {
                isInParent = true;
                break;
            }
        }
        
        // If not a parent method, add it at the end of the vtable
        if (!isInParent) {
            const auto& symbol = classInfo->getRegistryFunctionLocal()->get(methodName);
            if (!prysma::isa<SymbolFunctionLocal>(symbol.get())) {
                throw std::runtime_error("Error: Expected SymbolFunctionLocal");
            }
            auto* localSymbol = prysma::cast<SymbolFunctionLocal>(symbol.get()); 
            if (localSymbol->function != nullptr) {
                vtableElements.push_back(llvm::ConstantExpr::getBitCast(
                    localSymbol->function,
                    llvm::PointerType::get(llvm::Type::getInt8Ty(_contextGenCode->getBackend()->getContext()), 0)
                ));
                classInfo->getMethodIndices()[std::string(methodName)] = static_cast<unsigned int>(vtableElements.size() - 1);
            }
        }
    }
    
    // Create a constant array for the vtable
    llvm::ArrayType* vtableType = llvm::ArrayType::get(
        llvm::PointerType::get(llvm::Type::getInt8Ty(_contextGenCode->getBackend()->getContext()), 0),
        vtableElements.size()
    );
    llvm::Constant* vtableInitializer = llvm::ConstantArray::get(vtableType, vtableElements);
    
    // Create a global variable for the vtable
    llvm::Module* module = &(_contextGenCode->getBackend()->getModule());
    classInfo->setVTable(new llvm::GlobalVariable(
        *module,
        vtableType,
        true,  
        llvm::GlobalValue::LinkageTypes::InternalLinkage,
        vtableInitializer,
        "vtable_" + className
    )); 
}


void BuilderEnvironmentRegistryFunction::fill()
{   
    // 1. Fill global functions
    for(const auto& key : _contextGenCode->getRegistryFunctionGlobal()->getKeys())
    {
        const auto& oldSymbolUniquePtr = _contextGenCode->getRegistryFunctionGlobal()->get(key);
        const auto* oldSymbol = prysma::cast<const SymbolFunctionGlobal>(oldSymbolUniquePtr.get());
        
        if (oldSymbol->node == nullptr) { continue;}

        auto& oldSymbolNodeData = _contextGenCode->getNodeDataRegistry()->get(oldSymbol->node);

        llvm::Type* retType = oldSymbol->returnType->generateLLVMType(_contextGenCode->getBackend()->getContext());

        std::vector<llvm::Type*> paramTypes;
        for (auto* arg : oldSymbolNodeData.getArguments()) {
            auto* argFunction = prysma::cast<NodeArgFunction>(arg);
            auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(argFunction);

            paramTypes.push_back(nodeData.getType()->generateLLVMType(_contextGenCode->getBackend()->getContext()));
        }

        llvm::FunctionType* funcType = llvm::FunctionType::get(retType, paramTypes, false);
        
        llvm::Function* realFunction = llvm::Function::Create(
            funcType, 
            llvm::Function::ExternalLinkage, 
            key,          
            _contextGenCode->getBackend()->getModule()
        );

        auto newSymbol = std::make_unique<SymbolFunctionLocal>();
        newSymbol->function = realFunction;
        newSymbol->returnType = oldSymbol->returnType;
        newSymbol->node = oldSymbol->node;
        
        llvm::StringRef safeKey = oldSymbolNodeData.getName().value;
        _contextGenCode->getRegistryFunctionLocal()->registerElement(safeKey, std::move(newSymbol));        
    }

    // Fill class methods (VTable, mangling)
    for (const auto& className : _contextGenCode->getRegistryClass()->getKeys())
    {
        auto const& classInfo = _contextGenCode->getRegistryClass()->get(className);
        
        for (const auto& methodName : classInfo->getRegistryFunctionLocal()->getKeys())
        {
            // Retrieve the SymbolFunctionLocal created in FillingVisitorRegistry
            const auto& symbolUniquePtr = classInfo->getRegistryFunctionLocal()->get(methodName);
            auto* symbol = prysma::cast<SymbolFunctionLocal>(symbolUniquePtr.get());
            
            if (symbol->node == nullptr || symbol->function != nullptr) 
            {
                std::string errorMsg = "Error: SymbolFunctionLocal for method '";
                errorMsg += methodName;
                errorMsg += "' of class '";
                errorMsg += className;
                errorMsg += "' is not properly initialized or already processed";
                throw std::runtime_error(errorMsg);
            }

            llvm::Type* retType = symbol->returnType->generateLLVMType(_contextGenCode->getBackend()->getContext());
            
            std::vector<llvm::Type*> paramTypes;
            
            // Add the hidden 'this' parameter as the first parameter
            paramTypes.push_back(llvm::PointerType::getUnqual(_contextGenCode->getBackend()->getContext()));

            auto& symbolNodeData = _contextGenCode->getNodeDataRegistry()->get(symbol->node);

            for (auto* arg : symbolNodeData.getArguments()) {
                auto* argFunction = prysma::cast<NodeArgFunction>(arg);
                auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(argFunction);

                paramTypes.push_back(nodeData.getType()->generateLLVMType(_contextGenCode->getBackend()->getContext()));
            }

            llvm::FunctionType* funcType = llvm::FunctionType::get(retType, paramTypes, false);
            
            // Name mangling for LLVM (e.g., Player_initialize)
            std::string llvmName = className;
            llvmName += "_";
            llvmName += methodName;

            llvm::Function* realFunction = llvm::Function::Create(
                funcType, 
                llvm::Function::ExternalLinkage, 
                llvmName,          
                _contextGenCode->getBackend()->getModule()
            );
            
            symbol->function = realFunction;    
            
            // Build the vtable for this class
            std::vector<NodeDeclarationFunction*> parentMethodList;
            if (classInfo->getParentInheritance() != nullptr) {
                auto parentExtractor = MembersExtractorClass{ _contextGenCode };

                classInfo->getParentInheritance()->accept(&parentExtractor);
                parentMethodList = parentExtractor.getMethods();
            }
            if(methodName != className) // If the method is not the builder, put it in the vtable, otherwise do not put it in the vtable and call it directly by its mangled name
            {
                buildVTable(classInfo.get(), className, parentMethodList);
            }
            // Check if the class contains a builder, otherwise the class is not valid.
            if (!classInfo->getRegistryFunctionLocal()->exists(className)) {
                throw std::runtime_error("Error: The class '" + className + "' must have a builder with the same name");
            }
        }
    }
}
