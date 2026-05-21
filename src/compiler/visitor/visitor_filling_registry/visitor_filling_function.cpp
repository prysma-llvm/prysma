//===-- visitor_filling_function.cpp ----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/visitor/visitor_filling_registry/visitor_filling_registry.h"
#include "compiler/ast/registry/registry_class.h"
#include "compiler/ast/registry/registry_function.h"
#include "compiler/ast/registry/types/i_type.h"
#include <llvm/ADT/StringRef.h>
#include <memory>
#include <string>
#include <utility>

void FillingVisitorRegistry::visiter(NodeDeclarationFunction* nodeDeclarationFunction)
{
    auto& nodeDeclFuncData = _contextGenCode->getNodeDataRegistry()->get(nodeDeclarationFunction);

    IType* returnType = nodeDeclFuncData.getReturnType();
    llvm::StringRef functionName = nodeDeclFuncData.getName().value;
    
    if (_contextGenCode->getCurrentClassName() != "") {
        // class context (method)
        auto functionSymbol = std::make_unique<SymbolFunctionLocal>();
        functionSymbol->returnType = returnType;
        functionSymbol->node = nodeDeclarationFunction;

        std::string className = _contextGenCode->getCurrentClassName();
        auto* classInfo = _contextGenCode->getRegistryClass()->get(className).get();
        classInfo->getRegistryFunctionLocal()->registerElement(functionName, std::move(functionSymbol));
    } else {
        // global context (global function)
        auto functionSymbol = std::make_unique<SymbolFunctionGlobal>();
        functionSymbol->returnType = returnType;
        functionSymbol->node = nodeDeclarationFunction;

        _contextGenCode->getRegistryFunctionGlobal()->registerElement(std::string(functionName), std::move(functionSymbol));
    }
}
