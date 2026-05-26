//===-- parser_return.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef PARSER_RETURN_CPP
#define PARSER_RETURN_CPP

#include "compiler/function/parser_return.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_parser.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include <cstddef>
#include <vector>

ParserReturn::ParserReturn(ContextParser& contextParser) 
    : _contextParser(contextParser)
{}

ParserReturn::~ParserReturn() = default;

auto ParserReturn::parse(const std::vector<Token>& tokens, std::size_t& index) -> INode*
{
    consume(tokens, index, TOKEN_RETURN, "Error: not the correct token! 'return'");

    INode* returnValue = nullptr;

    if (index < tokens.size() && tokens[index].type != TOKEN_SEMICOLON) {
        returnValue = _contextParser.getBuilderTreeEquation()->build(tokens, index);
    } else {
        consume(tokens, index, TOKEN_SEMICOLON, "Error: semicolon expected after return");

        auto* new_node = _contextParser.getBuilderTreeEquation()->allocate<NodeReturn>(_contextParser.getIdGenerator()->next());
       _contextParser.getNodeDataRegistry()->construct(new_node, returnValue);

        return new_node;
    }
    
    consume(tokens, index, TOKEN_SEMICOLON, "Error: ';' expected at the end of return");

    auto* new_node = _contextParser.getBuilderTreeEquation()->allocate<NodeReturn>(_contextParser.getIdGenerator()->next());
    _contextParser.getNodeDataRegistry()->construct(new_node, returnValue);

    return new_node;
}

#endif /* PARSER_RETURN_CPP */




