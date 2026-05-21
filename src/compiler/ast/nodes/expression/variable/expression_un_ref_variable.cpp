//===-- expression_un_ref_variable.cpp --------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef EXPRESSION_UNREFVARIABLE_CPP
#define EXPRESSION_UNREFVARIABLE_CPP

#include "compiler/variable/expression_un_ref_variable.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include <stdexcept>
#include <vector>

ExpressionUnRefVariable::ExpressionUnRefVariable(ContextExpression& expressionContext)
    : _context(expressionContext)
{}

ExpressionUnRefVariable::~ExpressionUnRefVariable()
= default;

auto ExpressionUnRefVariable::build(std::vector<Token>& equation) -> INode*
{
    if (equation.size() < 2 || equation[1].type != TOKEN_IDENTIFIER) {
        throw std::runtime_error("Error: 'unref' must be followed by an identifier");
    }

    auto* nodeUnrefVar = _context.getBuilderTreeEquation()->allocate<NodeUnRefVariable>(_context.getIdGenerator()->next());
    _context.getNodeDataRegistry()->construct(nodeUnrefVar, equation[1]);

    return nodeUnrefVar;
}

#endif /* EXPRESSION_UNREFVARIABLE_CPP */
