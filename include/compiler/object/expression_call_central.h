//===-- expression_call_central.h -------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef EXPRESSION_CALLCENTRAL_H
#define EXPRESSION_CALLCENTRAL_H

#include "compiler/ast/nodes/expression/i_expression_engine.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/lexer/lexer.h"
#include <vector>

class ExpressionCallCentral : public IExpressionEngine
{
private:
    ContextExpression& _context;

public:
    explicit ExpressionCallCentral(ContextExpression& expressionContext);
    ~ExpressionCallCentral() override;

    ExpressionCallCentral(const ExpressionCallCentral&) = delete;
    auto operator=(const ExpressionCallCentral&) -> ExpressionCallCentral& = delete;

    ExpressionCallCentral(ExpressionCallCentral&&) = delete;
    auto operator=(ExpressionCallCentral&&) -> ExpressionCallCentral& = delete;

    auto build(const std::vector<Token>& equation) -> INode* override;
};

#endif /* EXPRESSION_CALLCENTRAL_H */
