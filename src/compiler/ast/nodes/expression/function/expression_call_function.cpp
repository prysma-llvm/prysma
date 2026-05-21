//===-- expression_call_function.cpp ----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef EXPRESSION_CALLFUNCTION_CPP
#define EXPRESSION_CALLFUNCTION_CPP

#include "compiler/function/expression_call_function.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/function/parser_call_function.h"
#include "compiler/lexer/lexer.h"
#include <vector>

ExpressionCallFunction::ExpressionCallFunction(ContextExpression& expressionContext)
    : _context(expressionContext)
{}

ExpressionCallFunction::~ExpressionCallFunction()
= default;

auto ExpressionCallFunction::build(std::vector<Token>& equation) -> INode*
{
    ParserCallFunction parserCall(*_context.getContextParser());
    std::size_t indexZero = 0;
    return parserCall.parse(equation, indexZero);
}

#endif /* EXPRESSION_CALLFUNCTION_CPP */
