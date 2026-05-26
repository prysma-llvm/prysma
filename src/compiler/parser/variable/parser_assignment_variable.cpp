//===-- parser_assignment_variable.cpp --------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef PARSER_ASSIGNMENTVARIABLE_CPP
#define PARSER_ASSIGNMENTVARIABLE_CPP

#include "compiler/variable/parser_assignment_variable.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_parser.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include <cstddef>
#include <string>
#include <vector>


ParserAssignmentVariable::ParserAssignmentVariable(ContextParser& contextParser) 
    : _contextParser(contextParser)
{}

ParserAssignmentVariable::~ParserAssignmentVariable()
= default;

auto ParserAssignmentVariable::parse(const std::vector<Token>& tokens, std::size_t& index) -> INode*
{
    consume(tokens, index, TOKEN_ASSIGN, "Error: 'aff' expected");
    Token nameToken = consume(tokens, index, TOKEN_IDENTIFIER, "Error: variable name expected");
    std::string variableName = std::string(nameToken.value);

    if (tokens[index].type == TOKEN_DOT) {
        consume(tokens, index, TOKEN_DOT, "Error '.'");
        Token attributeToken = consume(tokens, index, TOKEN_IDENTIFIER, "Error: attribute expected");
        variableName += "." + std::string(attributeToken.value);
    }

    INode* indexExpression = nullptr;

    if (tokens[index].type == TOKEN_BRACKET_OPEN) {
        consume(tokens, index, TOKEN_BRACKET_OPEN, "Error '['");
        indexExpression = _contextParser.getBuilderTreeEquation()->build(tokens, index);
        consume(tokens, index, TOKEN_BRACKET_CLOSE, "Error: ']' expected after index");
    }

    consume(tokens, index, TOKEN_EQUAL, "Error: '=' expected");
    INode* expression = _contextParser.getBuilderTreeEquation()->build(tokens, index);
    consume(tokens, index, TOKEN_SEMICOLON, "Error: ';' expected");

    Token constructedToken = nameToken;

    if (indexExpression != nullptr) {
        auto* nodeAssignmentArr = _contextParser.getBuilderTreeInstruction()->allocate<NodeAssignmentArray>(_contextParser.getIdGenerator()->next());
        _contextParser.getNodeDataRegistry()->construct(nodeAssignmentArr, constructedToken, indexExpression, expression, nameToken);

        return nodeAssignmentArr;
    }

    auto* nodeAssignmentVar = _contextParser.getBuilderTreeInstruction()->allocate<NodeAssignmentVariable>(_contextParser.getIdGenerator()->next());
    _contextParser.getNodeDataRegistry()->construct(nodeAssignmentVar, constructedToken, expression, nameToken);

    return nodeAssignmentVar;
}

#endif /* PARSER_ASSIGNMENTVARIABLE_CPP */




