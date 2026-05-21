//===-- parser_call_central.h -----------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef PARSER_CALLCENTRAL_H
#define PARSER_CALLCENTRAL_H

#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_parser.h"
#include "compiler/parser/interfaces/i_parser.h"
#include "compiler/lexer/lexer.h"
#include <vector>

class ParserCallCentral : public IParser
{
private:
    ContextParser& _contextParser;

public:
    explicit ParserCallCentral(ContextParser& contextParser);
    ~ParserCallCentral() override;

    ParserCallCentral(const ParserCallCentral&) = delete;
    auto operator=(const ParserCallCentral&) -> ParserCallCentral& = delete;
    ParserCallCentral(ParserCallCentral&&) = delete;
    auto operator=(ParserCallCentral&&) -> ParserCallCentral& = delete;
    
    auto parse(std::vector<Token>& tokens, std::size_t& index) -> INode* override;
};

#endif /* PARSER_CALLCENTRAL_H */
