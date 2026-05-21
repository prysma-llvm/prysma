//===-- context_expression.h ------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef A089019F_3756_4ED8_96C6_BBAA2C5A05F0
#define A089019F_3756_4ED8_96C6_BBAA2C5A05F0

#include "compiler/ast/registry/data/id_generator.hpp"
#include "compiler/ast/registry/data/node_data_registry.hpp"
#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/ast/interfaces/i_builder_tree.h"
#include "compiler/ast/registry/context_parser.h"
#include "compiler/ast/registry/registry_type.h"
#include "compiler/parser/parser_type.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include <llvm/Support/Allocator.h>
#include <stdexcept>

struct ContextExpression
{
private: // TODO: adapter le naming scheme vers _membre
    IBuilderTree* builderTreeEquation;
    IBuilderTree* builderTreeInstruction;
    TypeParser* parserType;
    ContextParser* contextParser;
    llvm::BumpPtrAllocator* arena;
    RegistryVariable* registryVariable;
    RegistryType* registryType;
    NodeDataRegistry* nodeDataRegistry;
    IdGenerator* idGenerator;

public:
    ContextExpression(
        IBuilderTree* p_builderTreeEquation,
        IBuilderTree* p_builderTreeInstruction,
        TypeParser* p_parserType,
        ContextParser* p_contextParser,
        llvm::BumpPtrAllocator* p_arena,
        RegistryVariable* p_registryVariable,
        RegistryType* p_registryType,
        NodeDataRegistry* p_nodeDataRegistry,
        IdGenerator* p_idGenerator
    )
        : builderTreeEquation(p_builderTreeEquation),
          builderTreeInstruction(p_builderTreeInstruction),
          parserType(p_parserType),
          contextParser(p_contextParser),
          arena(p_arena),
          registryVariable(p_registryVariable),
          registryType(p_registryType),
          nodeDataRegistry(p_nodeDataRegistry),
          idGenerator(p_idGenerator)
    {
        if (p_contextParser == nullptr)
        {
            throw std::invalid_argument("contextParser cannot be null");
        }
        if (p_builderTreeEquation == nullptr)
        {
            throw std::invalid_argument("builderTreeEquation cannot be null");
        }
        if (p_builderTreeInstruction == nullptr)
        {
            throw std::invalid_argument("builderTreeInstruction cannot be null");
        }
        if (p_parserType == nullptr)
        {
            throw std::invalid_argument("parserType cannot be null");
        }
        if (p_registryVariable == nullptr)
        {
            throw std::invalid_argument("registryVariable cannot be null");
        }
        if (p_registryType == nullptr)
        {
            throw std::invalid_argument("registryType cannot be null");
        }   
        if (p_nodeDataRegistry == nullptr)
        {
            throw std::invalid_argument("nodeDataRegistry cannot be null");
        }  
        if (p_idGenerator == nullptr)
        {
            throw std::invalid_argument("idGenerator cannot be null");
        }   
    }

    PRYSMA_NODISCARD auto getBuilderTreeEquation() const -> IBuilderTree* { return builderTreeEquation; }
    PRYSMA_NODISCARD auto getBuilderTreeInstruction() const -> IBuilderTree* { return builderTreeInstruction; }
    PRYSMA_NODISCARD auto getTypeParser() const -> TypeParser* { return parserType; }
    PRYSMA_NODISCARD auto getContextParser() const -> ContextParser* { return contextParser; }
    PRYSMA_NODISCARD auto getArena() const -> llvm::BumpPtrAllocator* { return arena; }
    PRYSMA_NODISCARD auto getRegistryVariable() const -> RegistryVariable* { return registryVariable; }
    PRYSMA_NODISCARD auto getRegistryType() const -> RegistryType* { return registryType; }
    PRYSMA_NODISCARD auto getNodeDataRegistry() const -> NodeDataRegistry* { return nodeDataRegistry; }
    PRYSMA_NODISCARD auto getIdGenerator() const -> IdGenerator* { return idGenerator; }
};

#endif /* A089019F_3756_4ED8_96C6_BBAA2C5A05F0 */
