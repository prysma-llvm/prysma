//===-- expression_negation.cpp ---------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef EXPRESSION_NEGATION_CPP
#define EXPRESSION_NEGATION_CPP

#include "compiler/math/expression_negation.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/lexer/lexer.h"
#include <stdexcept>
#include <vector>

ExpressionNegation::ExpressionNegation(ContextExpression& expressionContext)
    : _context(expressionContext)
{}

ExpressionNegation::~ExpressionNegation()
= default;

auto ExpressionNegation::build(const std::vector<Token>& equation) -> INode*
{
    if (equation.size() < 2) {
        throw std::runtime_error("Error: '!' must be followed by an expression");
    }

    std::vector<Token> operand(equation.begin() + 1, equation.end());
    INode* exprOperand = _context.getBuilderTreeEquation()->build(operand);

    auto* nodeNegation = _context.getBuilderTreeEquation()->allocate<NodeNegation>(_context.getIdGenerator()->next());
    _context.getNodeDataRegistry()->construct(nodeNegation, equation[0], exprOperand);

    return nodeNegation;
}

#endif /* EXPRESSION_NEGATION_CPP */
