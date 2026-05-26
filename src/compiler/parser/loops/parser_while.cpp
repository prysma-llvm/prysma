//===-- parser_while.cpp ----------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef PARSER_WHILE_CPP
#define PARSER_WHILE_CPP

#include "compiler/loops/parser_while.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_parser.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include "compiler/visitor/interfaces/i_visitor.h"
#include <cstddef>
#include <llvm/ADT/ArrayRef.h>
#include <vector>


ParserWhile::ParserWhile(ContextParser& contextParser) 
    : _contextParser(contextParser)
{}

ParserWhile::~ParserWhile() = default;

auto ParserWhile::parse(const std::vector<Token>& tokens, std::size_t& index) -> INode*
{
    consume(tokens, index, TOKEN_WHILE, "Error, expected token 'while' ");
    consume(tokens, index, TOKEN_PAREN_OPEN, "Error, token is not '('! ");
    
    INode* condition = _contextParser.getBuilderTreeEquation()->build(tokens, index);

    consume(tokens, index, TOKEN_PAREN_CLOSE, "Error, token is not ')'! ");
    consume(tokens, index, TOKEN_BRACE_OPEN, "Error, token is not '{'");

    auto blockWhileChildren = consumeChildBody(tokens, index, _contextParser.getBuilderTreeInstruction(), TOKEN_BRACE_CLOSE);

    auto* nodeBlockWhile = _contextParser.getBuilderTreeInstruction()->allocate<NodeInstruction>(_contextParser.getIdGenerator()->next()); 
    _contextParser.getNodeDataRegistry()->construct(nodeBlockWhile, blockWhileChildren);

    consume(tokens, index, TOKEN_BRACE_CLOSE, "Error, token is not '}'");

    auto* nodeBlockEndWhile = _contextParser.getBuilderTreeInstruction()->allocate<NodeInstruction>(_contextParser.getIdGenerator()->next()); 
    _contextParser.getNodeDataRegistry()->construct(nodeBlockEndWhile, llvm::ArrayRef<INode*>{});

    auto* nodeWhile = _contextParser.getBuilderTreeInstruction()->allocate<NodeWhile>(_contextParser.getIdGenerator()->next()); 
    _contextParser.getNodeDataRegistry()->construct(
        nodeWhile,
        condition, 
        nodeBlockWhile,
        nodeBlockEndWhile
    );

    return nodeWhile;
}

#endif /* PARSER_WHILE_CPP */




