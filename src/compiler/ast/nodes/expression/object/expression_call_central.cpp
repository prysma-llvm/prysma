//===-- expression_call_central.cpp -----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef EXPRESSION_CALLCENTRAL_CPP
#define EXPRESSION_CALLCENTRAL_CPP

#include "compiler/object/expression_call_central.h"

#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/function/parser_call_function.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include "compiler/object/parser_call_object.h"
#include <cstddef>
#include <vector>

ExpressionCallCentral::ExpressionCallCentral(ContextExpression& expressionContext)
    : _context(expressionContext)
{}

ExpressionCallCentral::~ExpressionCallCentral()
= default;

auto ExpressionCallCentral::build(std::vector<Token>& equation) -> INode*
{
    std::size_t indexZero = 0;

    const bool callObject = equation.size() >= 3
        && equation[0].type == TOKEN_CALL
        && equation[1].type == TOKEN_IDENTIFIER
        && equation[2].type == TOKEN_DOT;

    if (callObject) {
        ParserCallObject parserCall(*_context.getContextParser());
        return parserCall.parse(equation, indexZero);
    }

    ParserCallFunction parserCall(*_context.getContextParser());
    return parserCall.parse(equation, indexZero);
}

#endif /* EXPRESSION_CALLCENTRAL_CPP */
