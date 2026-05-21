//===-- parser_type.h -------------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef BA12D61E_9205_4816_94C1_379B8ABA63D3
#define BA12D61E_9205_4816_94C1_379B8ABA63D3

#include "compiler/ast/interfaces/i_builder_tree.h"
#include "compiler/ast/registry/data/node_data_registry.hpp"
#include "compiler/ast/registry/types/i_type.h"
#include "compiler/ast/registry/registry_type.h"
#include "compiler/lexer/lexer.h"
#include <cstddef>
#include <vector>

class TypeParser {
private:
    RegistryType* _registryType;
    IBuilderTree* _builderTree;
    NodeDataRegistry* _nodeDataRegistry;

public:
    TypeParser(RegistryType* registryType, NodeDataRegistry* nodeDataRegistry, IBuilderTree* builderTree);
    ~TypeParser() = default;

    TypeParser(const TypeParser&) = delete;
    auto operator=(const TypeParser&) -> TypeParser& = delete;
    TypeParser(TypeParser&&) = delete;
    auto operator=(TypeParser&&) -> TypeParser& = delete;
    auto parse(std::vector<Token>& tokens, std::size_t& index) -> IType*;
};

#endif /* BA12D61E_9205_4816_94C1_379B8ABA63D3 */
