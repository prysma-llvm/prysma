//===-- expression_call_object.cpp ------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef EXPRESSION_CALLOBJECT_CPP
#define EXPRESSION_CALLOBJECT_CPP

#include "compiler/object/expression_call_object.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/lexer/lexer.h"
#include "compiler/object/parser_call_object.h"
#include <cstddef>
#include <vector>

ExpressionCallObject::ExpressionCallObject(ContextExpression& expressionContext)
    : _context(expressionContext)
{}

ExpressionCallObject::~ExpressionCallObject()
= default;
// Example: call object.method() or call object.method(arg int64 param1, arg int64 param2)
auto ExpressionCallObject::build(std::vector<Token>& equation) -> INode*
{
    ParserCallObject parserCall(*_context.getContextParser());
    std::size_t indexZero = 0;
    return parserCall.parse(equation, indexZero);
}

#endif /* EXPRESSION_CALLOBJECT_CPP */
