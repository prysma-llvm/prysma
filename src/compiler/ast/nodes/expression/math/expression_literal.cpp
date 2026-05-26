//===-- expression_literal.cpp ----------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef EXPRESSION_LITERAL_CPP
#define EXPRESSION_LITERAL_CPP

#include "compiler/math/expression_literal.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/lexer/lexer.h"
#include <vector>

ExpressionLiteral::ExpressionLiteral(ContextExpression& expressionContext)
    : _context(expressionContext)
{}

ExpressionLiteral::~ExpressionLiteral()
= default;

auto ExpressionLiteral::build(const std::vector<Token>& equation) -> INode*
{
    auto* nodeLiteral = _context.getBuilderTreeEquation()->allocate<NodeLiteral>(_context.getIdGenerator()->next());

    _context.getNodeDataRegistry()->construct(nodeLiteral, equation[0]);

    return nodeLiteral;
}

#endif /* EXPRESSION_LITERAL_CPP */
