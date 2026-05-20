//===-- expression_identifiant.cpp ------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef EXPRESSION_IDENTIFIER_CPP
#define EXPRESSION_IDENTIFIER_CPP

#include "compiler/variable/expression_identifiant.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include <cstddef>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FormatVariadic.h>
#include <string.h>
#include <string>
#include <vector>
#include <cstring>

ExpressionIdentifiant::ExpressionIdentifiant(ContextExpression& expressionContext)
    : _context(expressionContext)
{}

ExpressionIdentifiant::~ExpressionIdentifiant()
= default;

auto ExpressionIdentifiant::build(std::vector<Token>& equation) -> INode*
{
    bool isArray = false;
    size_t bracketIndex = 0;
    std::vector<Token> indexEquation;

    for (size_t index = 0; index < equation.size(); index++) {
        if (equation[index].type == TOKEN_BRACKET_OPEN) {
            bracketIndex = index;
            isArray = true;
            break;
        }
    }

    if (isArray) {
        for (size_t index = bracketIndex; index < equation.size(); index++) {
            if (equation[index].type == TOKEN_BRACKET_CLOSE) {
                break;
            }
            if (index != bracketIndex) {
                indexEquation.push_back(equation[index]);
            }
        }

        INode* indexExpr = _context.getBuilderTreeEquation()->build(indexEquation);

        if (bracketIndex == 3 && equation[1].type == TOKEN_DOT) {
            Token combinedName;
            std::string tempStr = llvm::formatv("{0}.{1}", equation[0].value, equation[2].value).str();
            
            auto& arena = _context.getBuilderTreeEquation()->getArena();
            char* arr = static_cast<char*>(arena.Allocate(tempStr.size() + 1, 1));
            std::memcpy(arr, tempStr.c_str(), tempStr.size() + 1);

            combinedName.value = llvm::StringRef(arr, tempStr.size());
            combinedName.type = TOKEN_IDENTIFIER;

            auto* nodeReadingArr = _context.getBuilderTreeEquation()->allocate<NodeReadingArray>(_context.getIdGenerator()->next());
            _context.getNodeDataRegistry()->construct(nodeReadingArr, indexExpr, combinedName);

            return nodeReadingArr;
        }
        

        auto* nodeReadingArr = _context.getBuilderTreeEquation()->allocate<NodeReadingArray>(_context.getIdGenerator()->next());
        _context.getNodeDataRegistry()->construct(nodeReadingArr, indexExpr, equation[0]);

        return nodeReadingArr;
    }

    if (equation.size() == 3 && equation[1].type == TOKEN_DOT) {

        auto* nodeAccessAttr = _context.getBuilderTreeEquation()->allocate<NodeAccesAttribute>(_context.getIdGenerator()->next());
        _context.getNodeDataRegistry()->construct(nodeAccessAttr, equation[0], equation[2]);

        return nodeAccessAttr;
    }

    auto* nodeLiteral = _context.getBuilderTreeEquation()->allocate<NodeLiteral>(_context.getIdGenerator()->next());
    _context.getNodeDataRegistry()->construct(nodeLiteral, equation[0]);

    return nodeLiteral;
}

#endif /* EXPRESSION_IDENTIFIER_CPP */
