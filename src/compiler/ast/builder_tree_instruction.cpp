//===-- builder_tree_instruction.cpp ----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/builder_tree_instruction.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/data/id_generator.hpp"
#include "compiler/ast/registry/data/node_data_registry.hpp"
#include "compiler/ast/registry/registry_instruction.h"
#include "compiler/parser/interfaces/i_parser.h"
#include "compiler/manager_error.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include "llvm/ADT/SmallVector.h"
#include <cstddef>
#include <llvm-18/llvm/ADT/ArrayRef.h>
#include <llvm-18/llvm/IR/Instruction.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/FormatVariadic.h>
#include <vector>

BuilderTreeInstruction::BuilderTreeInstruction(RegistryInstruction* registryInstructions, NodeDataRegistry* nodeDataRegistry, IdGenerator* idGenerator, llvm::BumpPtrAllocator& arena)
    : _nodeDataRegistry(nodeDataRegistry), _idGenerator(idGenerator), _registryInstructions(registryInstructions), _arena(arena) {}

BuilderTreeInstruction::~BuilderTreeInstruction() = default;

auto BuilderTreeInstruction::build(std::vector<Token>& tokens, std::size_t& index) -> INode*
{
    if (!_registryInstructions->exists(tokens[index].type)) {
        throw CompilationError(llvm::formatv(
            "Unknown instruction: '{0}'", tokens[index].value).str(),
                Line(tokens[index].line),
                Column(tokens[index].column)
            );
    }
    IParser* parentNode = _registryInstructions->get(tokens[index].type);
    INode* child = parentNode->parse(tokens, index);
                                                                                                                                                                                                                                                                                                         
    return child;
}


auto BuilderTreeInstruction::build(std::vector<Token>& tokens) -> INode*
{
    std::size_t index = 0; 
    llvm::SmallVector<INode*, 64> children;
    
    while (index < tokens.size() && tokens[index].type != TOKEN_EOF) {
        INode* child = build(tokens, index);
        if (child != nullptr) {
            children.push_back(child);
        }
    }

    auto* new_node = allocate<NodeInstruction>(_idGenerator->next()); // un nouveau noeud d'instruction (INode*) est allocated dans le pool
    llvm::ArrayRef<INode*> arr_ref = allocateArray<INode*>(children); // les noeuds enfants (INode*) sont allocated dans le pool 

    _nodeDataRegistry->construct(new_node, arr_ref); // le llvm::ArrayRef est inséré dans le registre au ID du nouveau noeud

    return new_node;
}

auto BuilderTreeInstruction::getArena() -> llvm::BumpPtrAllocator&
{
    return _arena;
}
