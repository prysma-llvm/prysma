//===-- builder_equation_flottante.h ----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef FLOATEQUATIONBUILDER_H
#define FLOATEQUATIONBUILDER_H

#include <cstddef>
#include <llvm/Support/Allocator.h>
#include <memory>
#include <vector>

#include "compiler/ast/registry/data/id_generator.hpp"
#include "compiler/ast/registry/data/node_data_registry.hpp"
#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/ast/interfaces/i_builder_tree.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/parser/equation/chain_of_responsibility.h"
#include "compiler/parser/equation/service_parenthesis.h"
#include "compiler/parser/equation/manager_operator.h"
#include "compiler/ast/registry/registry_symbole.h"
#include "compiler/ast/registry/registry_expression.h"

struct Token;

class BuilderFloatEquation : public IBuilderTree
{
private:
    NodeDataRegistry* _nodeDataRegistry;
    IdGenerator* _idGenerator;

    RegistryExpression* _expressionRegistry;
    llvm::BumpPtrAllocator& _arena;
    std::unique_ptr<RegistrySymbol> _symbolRegistry;
    
    std::unique_ptr<OperatorManager> _additionManager;
    std::unique_ptr<OperatorManager> _subtractionManager;
    std::unique_ptr<OperatorManager> _multiplicationManager;
    std::unique_ptr<OperatorManager> _divisionManager;
    std::unique_ptr<OperatorManager> _moduloManager;
    std::unique_ptr<OperatorManager> _lessThanManager;
    std::unique_ptr<OperatorManager> _greaterThanManager;
    std::unique_ptr<OperatorManager> _lessThanOrEqualManager;
    std::unique_ptr<OperatorManager> _greaterThanOrEqualManager;
    std::unique_ptr<OperatorManager> _equalManager;
    std::unique_ptr<OperatorManager> _notEqualManager;
    std::unique_ptr<OperatorManager> _andManager;
    std::unique_ptr<OperatorManager> _orManager;

    std::unique_ptr<ParenthesisService> _parenthesisService;
    std::unique_ptr<ChainOfResponsibility> _chainOfResponsibility;
        
    IBuilderTree* _builderTree;

    void initializeRegistry();

public: 

    BuilderFloatEquation(RegistryExpression* expressionRegistry, NodeDataRegistry* nodeDataRegistry, IdGenerator* idGenerator, llvm::BumpPtrAllocator& arena);
    
    ~BuilderFloatEquation() override;

    BuilderFloatEquation(const BuilderFloatEquation&) = delete;
    auto operator=(const BuilderFloatEquation&) -> BuilderFloatEquation& = delete;
    BuilderFloatEquation(BuilderFloatEquation&&) = delete;
    auto operator=(BuilderFloatEquation&&) -> BuilderFloatEquation& = delete;

    auto build(std::vector<Token>& tokens) -> INode* override;
    auto build(std::vector<Token>& tokens, std::size_t& index) -> INode* override;
    auto getArena() -> llvm::BumpPtrAllocator& override;
    
    PRYSMA_NODISCARD auto getBuilderTree() const -> IBuilderTree*;
};

#endif /* FLOATEQUATIONBUILDER_H */
