//===-- visitor_filling_variable.cpp ----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/visitor/visitor_filling_registry/visitor_filling_registry.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/ast/registry/types/i_type.h"
#include "compiler/lexer/lexer.h"

// Currently unused, for the future

// This class is intended to fill the global variable registry located outside a function
void FillingVisitorRegistry::visiter(NodeDeclarationVariable* nodeDeclarationVariable)
{
    auto& nodeDeclVarData = _contextGenCode->getNodeDataRegistry()->get(nodeDeclarationVariable);

    Token token; 
    IType* type = nodeDeclVarData.getType();
    token.value = nodeDeclVarData.getName().value;
   
    _contextGenCode->getRegistryVariable()->registerVariable(token, Symbol(nullptr, type));
}
