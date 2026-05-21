//===-- expression_string.cpp -----------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef EXPRESSION_STRING_CPP
#define EXPRESSION_STRING_CPP

#include "compiler/math/expression_string.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include <cstring>
#include <llvm/ADT/SmallVector.h>
#include <cstddef>
#include <llvm/ADT/StringRef.h>
#include <stdexcept>
#include <string>
#include <vector>

ExpressionString::ExpressionString(ContextExpression& expressionContext)
    : _context(expressionContext)
{}

ExpressionString::~ExpressionString()
= default;

auto ExpressionString::build(std::vector<Token>& equation) -> INode*
{
    llvm::SmallVector<INode*, 16> stringElements;

    std::size_t index = 0;
    if (equation.empty() || equation[0].type != TOKEN_QUOTE) {
        throw std::runtime_error("Error: a string must start with a quote");
    }
    index++;

    if (index >= equation.size() || equation[index].type != TOKEN_IDENTIFIER) {
        throw std::runtime_error("Error: a string must be composed of alphanumeric characters");
    }
    Token str = equation[index];
    index++;

    if (index >= equation.size() || equation[index].type != TOKEN_QUOTE) {
        throw std::runtime_error("Error: a string must end with a quote");
    }

    for (std::size_t charIndex = 0; charIndex < str.value.size(); charIndex++) {
        int ascii = static_cast<unsigned char>(str.value[charIndex]);
        Token token;
        token.type = TOKEN_LIT_INT;
        std::string asciiStr = std::to_string(ascii);
        
        auto& arena = _context.getBuilderTreeEquation()->getArena();
        char* arr = static_cast<char*>(arena.Allocate(asciiStr.size() + 1, 1));
        std::memcpy(arr, asciiStr.c_str(), asciiStr.size() + 1);
        
        token.value = llvm::StringRef(arr, asciiStr.size());
        token.line = str.line;
        token.column = str.column;

        auto* nodeLiteral = _context.getBuilderTreeEquation()->allocate<NodeLiteral>(_context.getIdGenerator()->next());
        _context.getNodeDataRegistry()->construct(nodeLiteral, token);

        stringElements.push_back(nodeLiteral); 
    }

    Token tokenZero;
    tokenZero.type = TOKEN_LIT_INT;
    tokenZero.value = "0";
    tokenZero.line = str.line;
    tokenZero.column = str.column;

    auto* nodeLiteral = _context.getBuilderTreeEquation()->allocate<NodeLiteral>(_context.getIdGenerator()->next());
    _context.getNodeDataRegistry()->construct(nodeLiteral, tokenZero);

    stringElements.push_back(nodeLiteral); 

    auto* nodeArrayInit = _context.getBuilderTreeEquation()->allocate<NodeArrayInitialization>(_context.getIdGenerator()->next());
    _context.getNodeDataRegistry()->construct(nodeArrayInit, _context.getBuilderTreeEquation()->allocateArray<INode*>(stringElements));

    return nodeArrayInit;
}

#endif /* EXPRESSION_STRING_CPP */
