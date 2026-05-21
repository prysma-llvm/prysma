//===-- visitor_general_gen_code.cpp ----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_gen_code.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/ast/registry/types/type_complex.h"
#include "compiler/utils/prysma_cast.h"
#include "compiler/visitor/visitor_base_generale.h"
#include <cstddef>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/Casting.h>
#include <string>

GeneralVisitorGenCode::GeneralVisitorGenCode(ContextGenCode* contextGenCode, OrchestratorInclude* orchestratorInclude) 
: VisitorBaseGenerale(contextGenCode), _orchestratorInclude(orchestratorInclude)
{}

GeneralVisitorGenCode::~GeneralVisitorGenCode()
= default;

void GeneralVisitorGenCode::traverseChild(NodeInstruction* node)
{
    auto& component = _contextGenCode->getNodeDataRegistry()->get(node);
    
    for (const auto& child : component.getChildren()) {
        child->accept(this);
    }
}

auto GeneralVisitorGenCode::evaluateExpression(INode* expression) -> Symbol {
    if (expression != nullptr) {
        expression->accept(this);
        return _contextGenCode->getTemporaryValue();
    }
    Symbol empty;
    empty = Symbol(nullptr, nullptr, nullptr);
    return empty;
}

auto GeneralVisitorGenCode::getClassNameFromSymbol(const Symbol& objectSymbol) -> std::string {
    std::string className;
    if (objectSymbol.getType() != nullptr) {
        if (auto* typeComplex = prysma::dyn_cast<TypeComplex>(objectSymbol.getType())) {
            className = typeComplex->getClassName();
        }
    }

    constexpr size_t CLASS_PREFIX_LENGTH = 6;
    if (className.empty() && objectSymbol.getPointedElementType() != nullptr) {
        if (auto* structType = llvm::dyn_cast<llvm::StructType>(objectSymbol.getPointedElementType())) {
            llvm::StringRef structName = structType->getName();
            if (structName.starts_with("Class_")) {
                className = structName.drop_front(CLASS_PREFIX_LENGTH).str();
            } else {
                className = structName.str();
            }
        }
    }
    return className;
}

