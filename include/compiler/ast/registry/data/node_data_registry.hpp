//===-- node_data_registry.hpp ----------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include "compiler/ast/ast_genere.h"
#include "compiler/ast/registry/data/handle_providers.hpp"
#include "compiler/ast/registry/data/multi_storage_registry.hpp"
#include "compiler/ast/registry/data/linear_table.hpp"
#include "compiler/ast/registry/data/node_data.hpp"

using NodeDataRegistryTable = LinearTable<
    Entry<NodeInstruction,         InstructionNodeData>,
    Entry<NodeCallFunction,        FunctionCallNodeData>,
    Entry<NodeArgFunction,         FunctionArgNodeData>,
    Entry<NodeDeclarationFunction, FunctionDeclarationNodeData>,
    Entry<NodeReturn,              ReturnNodeData>,
    Entry<NodeAssignmentVariable,  VariableAssignmentNodeData>,
    Entry<NodeDeclarationVariable, VariableDeclarationNodeData>,
    Entry<NodeRefVariable,         VariableRefNodeData>,
    Entry<NodeUnRefVariable,       VariableUnrefNodeData>,
    Entry<NodeIdentifiant,         IdentifierNodeData>,
    Entry<NodeAssignmentArray,     ArrayAssignmentNodeData>,
    Entry<NodeArrayInitialization, ArrayInitializationNodeData>,
    Entry<NodeReadingArray,        ArrayReadingNodeData>,
    Entry<NodeClass,               ClassNodeData>,
    Entry<NodeCallObject,          ObjectCallNodeData>,
    Entry<NodeAccesAttribute,      AccessAttributeNodeData>,
    Entry<NodeDeclarationObject,   ObjectDeclarationNodeData>,
    Entry<NodeIf,                  IfNodeData>,
    Entry<NodeNew,                 NewNodeData>,
    Entry<NodeDelete,              DeleteNodeData>,
    Entry<NodeInclude,             IncludeNodeData>,
    Entry<NodeWhile,               WhileNodeData>,
    Entry<NodeOperation,           OperationNodeData>,
    Entry<NodeLiteral,             LiteralNodeData>,
    Entry<NodeNegation,            NegationNodeData>,
    Entry<NodeString,              StringNodeData>
>;

using NodeDataRegistry = MultiStorageRegistry<NodeDataRegistryTable, NodeHandleProvider, 1 << 27>; // [16 777 216 / N table entries]
