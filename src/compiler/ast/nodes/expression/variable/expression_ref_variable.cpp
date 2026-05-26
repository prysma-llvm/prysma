//===-- expression_ref_variable.cpp -----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef EXPRESSION_REFVARIABLE_CPP
#define EXPRESSION_REFVARIABLE_CPP

#include "compiler/variable/expression_ref_variable.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include <cstddef>
#include <stdexcept>
#include <vector>

ExpressionRefVariable::ExpressionRefVariable(ContextExpression& expressionContext)
    : _context(expressionContext)
{}

ExpressionRefVariable::~ExpressionRefVariable()
= default;

auto ExpressionRefVariable::build(const std::vector<Token>& equation) -> INode*
{
    if (equation.size() < 2 || equation[1].type != TOKEN_IDENTIFIER) {
        throw std::runtime_error("Error: 'ref' must be followed by an identifier");
    }

    auto* nodeRefVar = _context.getBuilderTreeEquation()->allocate<NodeRefVariable>(_context.getIdGenerator()->next());
    _context.getNodeDataRegistry()->construct(nodeRefVar, equation[1]);

    return nodeRefVar;
}

#endif /* EXPRESSION_REFVARIABLE_CPP */
