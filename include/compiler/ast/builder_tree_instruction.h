//===-- builder_tree_instruction.h ------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef BDC39C44_6952_4793_8198_C083B106A089
#define BDC39C44_6952_4793_8198_C083B106A089

#include "compiler/ast/interfaces/i_builder_tree.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/data/id_generator.hpp"
#include "compiler/ast/registry/data/node_data_registry.hpp"
#include "compiler/lexer/lexer.h"
#include "compiler/ast/registry/registry_instruction.h"
#include <cstddef>
#include <llvm/Support/Allocator.h>
#include <vector>

class BuilderTreeInstruction : public IBuilderTree
{
private: 
    NodeDataRegistry* _nodeDataRegistry;
    IdGenerator* _idGenerator;

    RegistryInstruction* _registryInstructions;
    llvm::BumpPtrAllocator& _arena;

public: 

    BuilderTreeInstruction(RegistryInstruction* registryInstructions, NodeDataRegistry* _nodeDataRegistry, IdGenerator* idGenerator, llvm::BumpPtrAllocator& arena);
    ~BuilderTreeInstruction() override;

    // Delete copy and move constructors and assignment operators
    BuilderTreeInstruction(const BuilderTreeInstruction&) = delete;
    auto operator=(const BuilderTreeInstruction&) -> BuilderTreeInstruction& = delete;
    BuilderTreeInstruction(BuilderTreeInstruction&&) = delete;
    auto operator=(BuilderTreeInstruction&&) -> BuilderTreeInstruction& = delete;

    auto build(std::vector<Token>& tokens) -> INode* override;  
    auto build(std::vector<Token>& tokens, std::size_t& index) -> INode* override;
    auto getArena() -> llvm::BumpPtrAllocator& override;
};

#endif /* BDC39C44_6952_4793_8198_C083B106A089 */
