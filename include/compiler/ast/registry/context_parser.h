//===-- context_parser.h ----------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef F4099BCE_4896_40B4_B34C_93B4827706C3
#define F4099BCE_4896_40B4_B34C_93B4827706C3

#include "compiler/ast/registry/data/id_generator.hpp"
#include "compiler/ast/registry/data/node_data_registry.hpp"
#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/ast/interfaces/i_builder_tree.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/ast/registry/registry_type.h"
#include "compiler/parser/parser_type.h"
#include <stdexcept>

struct ContextParser
{
private: // TODO: adapter le naming scheme pour _membre
    IBuilderTree* builderTreeEquation;
    IBuilderTree* builderTreeInstruction;
    TypeParser* parserType;
    RegistryVariable* registryVariable;
    RegistryType* registryType;

    NodeDataRegistry* nodeDataRegistry;
    IdGenerator* idGenerator;

public:
    struct Dependencies {
        IBuilderTree* builderTreeEquation;
        IBuilderTree* builderTreeInstruction;
        TypeParser* parserType;
        RegistryVariable* registryVariable;
        RegistryType* registryType;
        NodeDataRegistry* nodeDataRegistry;
        IdGenerator* idGenerator;
    };

    explicit ContextParser(const Dependencies& deps)
        : builderTreeEquation(deps.builderTreeEquation),
          builderTreeInstruction(deps.builderTreeInstruction),
          parserType(deps.parserType),
          registryVariable(deps.registryVariable),
          registryType(deps.registryType),
          nodeDataRegistry(deps.nodeDataRegistry),
          idGenerator(deps.idGenerator)
    {
        if(builderTreeEquation == nullptr)
        {
            throw std::invalid_argument("builderTreeEquation cannot be null");
        }

        if(builderTreeInstruction == nullptr)
        {
            throw std::invalid_argument("builderTreeInstruction cannot be null");
        }

        if(parserType == nullptr)
        {
            throw std::invalid_argument("parserType cannot be null");
        }

        if(registryVariable == nullptr)
        {
            throw std::invalid_argument("registryVariable cannot be null");
        }

        if(registryType == nullptr)
        {
            throw std::invalid_argument("registryType cannot be null");
        }

        if(nodeDataRegistry == nullptr)
        {
            throw std::invalid_argument("nodeDataRegistry cannot be null");
        }

        if(idGenerator == nullptr)
        {
            throw std::invalid_argument("idGenerator cannot be null");
        }
    }    

    PRYSMA_NODISCARD auto getBuilderTreeEquation() const -> IBuilderTree* { return builderTreeEquation; }
    PRYSMA_NODISCARD auto getBuilderTreeInstruction() const -> IBuilderTree* { return builderTreeInstruction; }
    PRYSMA_NODISCARD auto getTypeParser() const -> TypeParser* { return parserType; }
    PRYSMA_NODISCARD auto getRegistryVariable() const -> RegistryVariable* { return registryVariable; }
    PRYSMA_NODISCARD auto getRegistryType() const -> RegistryType* { return registryType; }
    PRYSMA_NODISCARD auto getNodeDataRegistry() const -> NodeDataRegistry* { return nodeDataRegistry; }
    PRYSMA_NODISCARD auto getIdGenerator() const -> IdGenerator* { return idGenerator; }
};

#endif /* F4099BCE_4896_40B4_B34C_93B4827706C3 */
