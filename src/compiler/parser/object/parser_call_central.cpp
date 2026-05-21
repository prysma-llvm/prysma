//===-- parser_call_central.cpp ---------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef PARSER_CALLCENTRAL_CPP
#define PARSER_CALLCENTRAL_CPP

#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/function/parser_call_function.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include "compiler/object/parser_call_object.h"

#include "compiler/object/parser_call_central.h"

#include "compiler/ast/registry/context_parser.h"
#include <cstddef>
#include <vector>

ParserCallCentral::ParserCallCentral(ContextParser& contextParser)
    : _contextParser(contextParser)
{}

ParserCallCentral::~ParserCallCentral()
= default;

INode* ParserCallCentral::parse(std::vector<Token>& tokens, std::size_t& index)
{
    const std::size_t currentIndex = index;
    const bool callObject = currentIndex + 2 < tokens.size()
        && tokens[currentIndex].type == TOKEN_CALL
        && tokens[currentIndex + 1].type == TOKEN_IDENTIFIER
        && tokens[currentIndex + 2].type == TOKEN_DOT;

    if (callObject) {
        ParserCallObject parserObject(_contextParser);
        return parserObject.parse(tokens, index);
    }

    ParserCallFunction parserFunction(_contextParser);
    return parserFunction.parse(tokens, index);
}

#endif /* PARSER_CALLCENTRAL_CPP */
