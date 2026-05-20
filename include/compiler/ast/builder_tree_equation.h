//===-- builder_tree_equation.h ---------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include "compiler/ast/interfaces/i_builder_tree.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/data/node_data_registry.hpp"
#include "compiler/ast/registry/registry_symbole.h"
#include "compiler/ast/registry/registry_expression.h"
#include "compiler/lexer/lexer.h"
#include "compiler/parser/equation/chain_of_responsibility.h"
#include "compiler/parser/equation/interfaces/i_manager_parenthesis.h"
#include <cstddef>
#include <llvm/Support/Allocator.h>
#include <vector>


class BuilderTreeEquation : public IBuilderTree
{
private:
    NodeDataRegistry* _nodeDataRegistry;// TODO: à possiblement changer pour un ExpressionDataRegistry

    ChainOfResponsibility* _chainOfResponsibility;
    RegistrySymbol* _symbolRegistry;
    RegistryExpression* _expressionRegistry;
    IManagerParenthesis* _parenthesisManager;
    llvm::BumpPtrAllocator& _arena;
    Token _lastToken;

public:
    BuilderTreeEquation(
        NodeDataRegistry* nodeDataRegistry,
        ChainOfResponsibility* chainOfResponsibility,
        RegistrySymbol* symbolRegistry,
        RegistryExpression* expressionRegistry,
        IManagerParenthesis* parenthesisManager,
        llvm::BumpPtrAllocator& arena
    );
    
    auto build(std::vector<Token> &tokens) -> INode* override;
    auto build(std::vector<Token>& tokens, std::size_t& index) -> INode* override;
    auto getArena() -> llvm::BumpPtrAllocator& override;
};
