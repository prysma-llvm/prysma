//===-- i_expression.h ------------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef C2F85AFD_7B01_4D87_8D01_5C59514D2B2E
#define C2F85AFD_7B01_4D87_8D01_5C59514D2B2E

#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/lexer/lexer.h"
#include <vector>

class IExpression
{
public:
    virtual ~IExpression() = default;
    IExpression(const IExpression&) = delete;
    auto operator=(const IExpression&) -> IExpression& = delete;
    IExpression(IExpression&&) = delete;
    auto operator=(IExpression&&) -> IExpression& = delete;
    virtual auto build(const std::vector<Token>& equation) -> INode* = 0;
};

#endif /* C2F85AFD_7B01_4D87_8D01_5C59514D2B2E */
