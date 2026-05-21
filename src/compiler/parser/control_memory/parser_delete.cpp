//===-- parser_delete.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef PARSER_DELETE_CPP
#define PARSER_DELETE_CPP

#include "compiler/control_memory/parser_delete.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_parser.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include <cstddef>
#include <vector>


ParserDelete::ParserDelete(ContextParser& contextParser) 
    : _contextParser(contextParser)
{}

ParserDelete::~ParserDelete()
= default;

// Example: delete variableName;
auto ParserDelete::parse(std::vector<Token>& tokens, std::size_t& index) -> INode*
{
    consume(tokens, index, TOKEN_DELETE, "Expected 'delete' at the beginning of the delete instruction.");
    Token identifierToken = consume(tokens, index, TOKEN_IDENTIFIER, "Expected an identifier after 'delete'.");
    consume(tokens, index, TOKEN_SEMICOLON, "Expected ';' after the identifier in the delete instruction.");

    auto* nodeDelete = _contextParser.getBuilderTreeInstruction()->allocate<NodeDelete>(_contextParser.getIdGenerator()->next()); 
    _contextParser.getNodeDataRegistry()->construct(nodeDelete, identifierToken);

    return nodeDelete;  
}

#endif /* PARSER_DELETE_CPP */




