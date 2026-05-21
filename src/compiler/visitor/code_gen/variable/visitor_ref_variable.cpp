//===-- visitor_ref_variable.cpp --------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/lexer/lexer.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include <llvm/IR/Value.h>

void GeneralVisitorGenCode::visiter(NodeRefVariable* nodeRefVariable) 
{
    auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(nodeRefVariable);
    const auto& nodeToken = nodeData.getName();

    Symbol symbol = _contextGenCode->getRegistryVariable()->getVariable(nodeToken);
    llvm::Value* variable = symbol.getAddress();
    
    _contextGenCode->setTemporaryValue(Symbol(variable, _contextGenCode->getTemporaryValue().getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
    _contextGenCode->setTemporaryValue(Symbol(_contextGenCode->getTemporaryValue().getAddress(), symbol.getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
}
