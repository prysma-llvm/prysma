//===-- i_parser.h ----------------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef A32B33F7_0073_416C_873A_6FF7F40419F8
#define A32B33F7_0073_416C_873A_6FF7F40419F8

#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/lexer/lexer.h"
#include <cstddef>
#include <vector>

class IParser
{
public: 
    IParser() = default;
    virtual ~IParser() = default;
    IParser(const IParser&) = delete;
    auto operator=(const IParser&) -> IParser& = delete;
    IParser(IParser&&) = delete;
    auto operator=(IParser&&) -> IParser& = delete;
    virtual auto parse(std::vector<Token>& tokens, std::size_t& index) -> INode* = 0;
};

#endif /* A32B33F7_0073_416C_873A_6FF7F40419F8 */
