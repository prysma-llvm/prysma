//===-- visitor_filling_body_class.cpp --------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/visitor/visitor_filling_body_class/visitor_filling_body_class.h"
#include "compiler/ast/registry/context_gen_code.h"
#include "compiler/ast/registry/registry_class.h"
#include "compiler/ast/registry/registry_function.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/visitor/visitor_base_generale.h"
#include "compiler/visitor/extractors/members_extractor_class.h"
#include "compiler/utils/prysma_cast.h"
#include <llvm-18/llvm/IR/Function.h>
#include <llvm-18/llvm/IR/GlobalVariable.h>
#include <llvm-18/llvm/IR/Metadata.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/Support/FormatVariadic.h>
#include <stdexcept>
#include <string>
#include <vector>

FillingVisitorBodyClass::FillingVisitorBodyClass(ContextGenCode* contextGenCode)
    : VisitorBaseGenerale(contextGenCode)
{
}

FillingVisitorBodyClass::~FillingVisitorBodyClass()
= default;

void FillingVisitorBodyClass::visiter(NodeClass* nodeClass)
{
    auto& nodeClassData = _contextGenCode->getNodeDataRegistry()->get(nodeClass);

    std::string className = std::string(nodeClassData.getName().value);

    auto classExtractor = MembersExtractorClass{ _contextGenCode };

    for (INode* member : nodeClassData.getMembers()) {
        if (member != nullptr) {
            member->accept(&classExtractor);
        }
    }

    // 1. Retrieve the class object
    Class* classInfo = _contextGenCode->getRegistryClass()->get(className).get();

    // 2. Recursive resolution of inheritance
    INode* parentHeritage = classInfo->getParentInheritance();
    if(parentHeritage != nullptr)
    {
        parentHeritage->accept(this);
    }

    // 3. Initialize the layout (type vector)
    std::vector<llvm::Type*> classBodyElements;

    // Make the vpointer at address zero
    classBodyElements.push_back(llvm::PointerType::get(
        llvm::Type::getInt8Ty(
        _contextGenCode->getBackend()->getContext()),
         0
        ));

    // 4. Inherit parent fields and build the VTable
    if(parentHeritage != nullptr)
    {
        auto parentExtractor = MembersExtractorClass{ _contextGenCode };

        parentHeritage->accept(&parentExtractor);
        std::string parentName = parentExtractor.getClassName();

        Class* parentClassInfo = _contextGenCode->getRegistryClass()->get(parentName).get();

        // Extract parent methods to bind them at the same vtable position
        auto parentMethodKeys = parentClassInfo->getRegistryFunctionLocal()->getKeys();
      
        std::vector<NodeDeclarationFunction*> parentMethodList;
        for (const auto& key : parentMethodKeys) {
            const auto& symbol = parentClassInfo->getRegistryFunctionLocal()->get(key);
            if (!prysma::isa<SymbolFunctionLocal>(symbol.get())) {
                throw std::runtime_error("Error: SymbolFunctionLocal expected");
            }
            auto* localSymbol = prysma::cast<SymbolFunctionLocal>(symbol.get());
            if (localSymbol->node != nullptr) {
                auto methodExtractor = MembersExtractorClass{ _contextGenCode };

                localSymbol->node->accept(&methodExtractor);
                if (!methodExtractor.getMethods().empty()) {
                    parentMethodList.push_back(methodExtractor.getMethods()[0]);
                }
            }
        }

        // Check if parent methods exist in the current class, if not it's an error

        // Ici, nous retrouvons une manifestation de l'un des désavantages de cette nouvelle architecture.
        // L'indirection et le chasing causée par le lookup des données est très inéfficace lors de boucles 
        // comme celle-ci. À chaque itération, il y a une latence, ce qui rends impossible certaines
        // optimisations par exemple concernant la localité du cache.

        // Il s'agirait de trouver une optimisation spécifique au problème. Par exemple, il serait bien plus efficace
        // d'aller chercher toutes les composantes à l'avance avant la boucle afin d'éliminer l'indirection pendant la boucle.
        for(NodeDeclarationFunction* parentMethod : parentMethodList)
        {
            auto& parentNodeData = _contextGenCode->getNodeDataRegistry()->get(parentMethod);

            bool found = false;
            for(NodeDeclarationFunction* classMethod : classExtractor.getMethods())
            {
                auto& nodeDeclFuncData = _contextGenCode->getNodeDataRegistry()->get(classMethod);

                if(nodeDeclFuncData.getName().value == parentNodeData.getName().value)
                {
                    found = true;
                    break;
                }
            }

            if(!found)
            {
                throw std::runtime_error(
                    llvm::formatv(
                        "Class {0} must implement the method {1} inherited from {2}",
                        className, parentNodeData.getName().value, parentName
                    ).str()
                );
            }
        }
    }
   
    // 5. Extract variables from the AST
    // The index starts at 1 because index 0 is reserved for the vtable pointer
    unsigned int currentIndex = 1;
    
    // Traverse members to get variables in declaration order
    for(NodeDeclarationVariable* declarationVariable : classExtractor.getVariables())
    {
        auto& nodeDeclVarData = _contextGenCode->getNodeDataRegistry()->get(declarationVariable);

        IType* itype = nodeDeclVarData.getType();
        llvm::Type* variableType = itype->generateLLVMType(_contextGenCode->getBackend()->getContext());
        if (variableType != nullptr) {
            classBodyElements.push_back(variableType);
            // Register the index for Pass 3
            classInfo->getMemberIndices()[std::string(nodeDeclVarData.getName().value)] = currentIndex;
            currentIndex++;
        }
    }

    // 6. Seal the structure (setBody)
    if (classInfo->getStructType() != nullptr) {
        classInfo->getStructType()->setBody(classBodyElements);
    }
}
