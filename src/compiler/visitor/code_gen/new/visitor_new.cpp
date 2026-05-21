//===-- visitor_new.cpp -----------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/ast/registry/registry_class.h"
#include "compiler/ast/registry/registry_function.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/visitor/code_gen/helper/error_helper.h"
#include "compiler/utils/prysma_cast.h"
#include <cstddef>
#include <cstdint>
#include <llvm-18/llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/FormatVariadic.h>
#include <stdexcept>
#include <string>
#include <vector>

void GeneralVisitorGenCode::visiter(NodeNew* nodeNew)
{
    auto& module = _contextGenCode->getBackend()->getModule();
    auto& builder = _contextGenCode->getBackend()->getBuilder();

    auto& nodeNewData = _contextGenCode->getNodeDataRegistry()->get(nodeNew);

    llvm::Type* targetType = nullptr;
    Class* classInfo = nullptr;

    if (nodeNewData.getName().type == TOKEN_IDENTIFIER) {
        classInfo = _contextGenCode->getRegistryClass()->get(std::string(nodeNewData.getName().value)).get();
        classInfo = ErrorHelper::verifyNotNull(classInfo, llvm::formatv("Class '{0}' not found", nodeNewData.getName().value).str());
        targetType = classInfo->getStructType();
    } else {
        targetType = _contextGenCode->getRegistryType()->get(nodeNewData.getName().type);
    }

    targetType = ErrorHelper::verifyNotNull(targetType, "Target type not determined for 'new'");

    // LLVM decides if an int32 is 4 bytes, etc.
    const llvm::DataLayout& dataLayout = module.getDataLayout();
    uint64_t byteSize = dataLayout.getTypeAllocSize(targetType);
    
    // Convert this number to an LLVM value (i64) for malloc argument
    llvm::Value* llvmSize = builder.getInt64(byteSize);

    llvm::Function* mallocFunc = module.getFunction("prysma_malloc");

    mallocFunc = ErrorHelper::verifyNotNull(mallocFunc, "Function prysma_malloc not declared in the module");

    llvm::Value* allocatedAddress = builder.CreateCall(mallocFunc, {llvmSize}, "memory_new");

    // Fill the builder's argument vector with the arguments from nodeNew, e.g., new MyClass(arg1, arg2)
    std::vector<llvm::Value*> builderArgs;
    builderArgs.push_back(allocatedAddress);  // this

    // Add arguments passed to new (the node's children)
    for (INode* arg : nodeNewData.getArguments()) {
        arg->accept(this);  // Evaluate the expression (e.g., int = 204)
        builderArgs.push_back(_contextGenCode->getTemporaryValue().getAddress());
    }
    
    if (classInfo != nullptr && classInfo->getVTable() != nullptr) {
        // Initialize the vptr at address 0 of the allocated object
        llvm::Value* vptrAddress = builder.CreateStructGEP(targetType, allocatedAddress, 0, "vptr_address");
        llvm::Value* vtablePtr = builder.CreateBitCast(classInfo->getVTable(), builder.getPtrTy());
        builder.CreateStore(vtablePtr, vptrAddress);
    }

    if (classInfo != nullptr) {
        for (const auto& pair : classInfo->getMemberInitializers()) {
            const std::string& memberName = pair.first;
            INode* initExpression = pair.second;

            if (initExpression != nullptr) {
                auto* arrayInit = prysma::dyn_cast<NodeArrayInitialization>(initExpression);
                
                if (arrayInit != nullptr) {
                    if (classInfo->getMemberIndices().find(memberName) != classInfo->getMemberIndices().end()) {
                        unsigned int idx = classInfo->getMemberIndices()[memberName];
                        
                        Token memberToken; memberToken.value = memberName;
                        Symbol model = classInfo->getRegistryVariable()->getVariable(memberToken);
                        llvm::Type* memberType = model.getType()->generateLLVMType(_contextGenCode->getBackend()->getContext());
                        auto* typeArrayLLVM = llvm::dyn_cast<llvm::ArrayType>(memberType);
                        if (typeArrayLLVM != nullptr) {
                            llvm::Type* elementType = typeArrayLLVM->getElementType();
                            llvm::Value* memberPtr = builder.CreateStructGEP(targetType, allocatedAddress, idx, memberName + "_ptrinit");
                            
                            auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(arrayInit);
                            auto nodeElements = nodeData.getElements();

                            for (std::size_t i = 0; i < nodeElements.size(); ++i) {
                                std::vector<llvm::Value*> indices = {
                                    builder.getInt32(0),
                                    builder.getInt32(static_cast<uint32_t>(i))
                                }; 
                                llvm::Value* ptrElement = builder.CreateGEP(typeArrayLLVM, memberPtr, indices, "ptr_element");
                    
                                INode* element = nodeElements[i];
                                element->accept(this);
                                llvm::Value* expressionVal = _contextGenCode->getTemporaryValue().getAddress();
                                
                                llvm::Value* castedValue = _contextGenCode->getBackend()->createAutoCast(expressionVal, elementType);
                                builder.CreateStore(castedValue, ptrElement);
                            }
                        }
                    }
                } else {
                    initExpression->accept(this);
                    llvm::Value* calculatedValue = _contextGenCode->getTemporaryValue().getAddress();

                    if (calculatedValue != nullptr && classInfo->getMemberIndices().find(memberName) != classInfo->getMemberIndices().end()) {
                        unsigned int idx = classInfo->getMemberIndices()[memberName];
                        
                        Token memberToken; memberToken.value = memberName;
                        Symbol model = classInfo->getRegistryVariable()->getVariable(memberToken);
                        llvm::Type* memberType = model.getType()->generateLLVMType(_contextGenCode->getBackend()->getContext());

                        llvm::Value* castedValue = _contextGenCode->getBackend()->createAutoCast(calculatedValue, memberType);
                        llvm::Value* memberPtr = builder.CreateStructGEP(targetType, allocatedAddress, idx, memberName + "_ptrinit");
                        builder.CreateStore(castedValue, memberPtr);
                    }
                }
            }
        }
    }

    // Build the builder with arguments
    if (classInfo != nullptr) {
        llvm::StringRef builderName = nodeNewData.getName().value;
        if (classInfo->getRegistryFunctionLocal()->exists(std::string(builderName))) {
            const auto& symbolPtr = classInfo->getRegistryFunctionLocal()->get(std::string(builderName));
            if (!prysma::isa<SymbolFunctionLocal>(symbolPtr.get())) {
                throw std::runtime_error("Error: SymbolFunctionLocal expected");
            }
            const auto* functionSymbol = prysma::cast<const SymbolFunctionLocal>(symbolPtr.get());
            builder.CreateCall(functionSymbol->function, builderArgs);
        }
    }

    _contextGenCode->setTemporaryValue(Symbol(allocatedAddress, _contextGenCode->getTemporaryValue().getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
    _contextGenCode->setTemporaryValue(Symbol(_contextGenCode->getTemporaryValue().getAddress(), _contextGenCode->getTemporaryValue().getType(), targetType));
}
