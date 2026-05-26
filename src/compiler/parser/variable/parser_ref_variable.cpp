//===-- parser_ref_variable.cpp ---------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef PARSER_REFVARIABLE_CPP
#define PARSER_REFVARIABLE_CPP

#include "compiler/variable/parser_ref_variable.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_parser.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include <cstddef>
#include <string>
#include <vector>


ParserRefVariable::ParserRefVariable(ContextParser& contextParser) 
    : _contextParser(contextParser)
{}

ParserRefVariable::~ParserRefVariable()
=default;

// Example: ref variable
auto ParserRefVariable::parse(const std::vector<Token>& tokens, std::size_t& index) -> INode* 
{
    consume(tokens, index, TOKEN_REF, "Error: 'ref' expected");
    
    Token nameToken = consume(tokens, index, TOKEN_IDENTIFIER, "Error: variable name expected after 'ref'");

    auto* new_node = _contextParser.getBuilderTreeEquation()->allocate<NodeRefVariable>(_contextParser.getIdGenerator()->next());
    _contextParser.getNodeDataRegistry()->construct(new_node, nameToken);
    
    return new_node;
}

#endif /* PARSER_REFVARIABLE_CPP */




