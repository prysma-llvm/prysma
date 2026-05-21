//===-- parser_call_object.cpp ----------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef PARSER_CALLOBJECT_CPP
#define PARSER_CALLOBJECT_CPP

#include "compiler/object/parser_call_object.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_parser.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include <cstddef>
#include <vector>


ParserCallObject::ParserCallObject(ContextParser& contextParser) 
    : _contextParser(contextParser)
{}

ParserCallObject::~ParserCallObject()
= default;

auto ParserCallObject::parse(std::vector<Token>& tokens, std::size_t& index) -> INode*
{
  const bool callAsInstruction = index == 0 || tokens[index - 1].type != TOKEN_EQUAL;

  consume(tokens, index, TOKEN_CALL, "Error: 'call' expected");
  Token objectName = consume(tokens, index, TOKEN_IDENTIFIER, "Error: object identifier expected");
  consume(tokens, index, TOKEN_DOT, "Error: '.' expected after object name");
  Token methodName = consume(tokens, index, TOKEN_IDENTIFIER, "Error: method identifier expected");
  consume(tokens, index, TOKEN_PAREN_OPEN, "Error: '(' expected");

  auto children = consumeChildBody(tokens, index, _contextParser.getBuilderTreeEquation(), TOKEN_PAREN_CLOSE);

    auto* nodeCall = _contextParser.getBuilderTreeInstruction()->allocate<NodeCallObject>(_contextParser.getIdGenerator()->next()); 
    _contextParser.getNodeDataRegistry()->construct(nodeCall, objectName, methodName, children);

  consume(tokens, index, TOKEN_PAREN_CLOSE, "Error: ')' expected");

  if (callAsInstruction && index < tokens.size() && tokens[index].type == TOKEN_SEMICOLON) {
      consume(tokens, index, TOKEN_SEMICOLON, "Error: ';' expected at the end of the object call");
  }

  return nodeCall;
}

#endif /* PARSER_CALLOBJECT_CPP */




