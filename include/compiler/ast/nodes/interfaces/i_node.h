//===-- i_node.h ------------------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cstddef>
#include <llvm/IR/Value.h>
#include "compiler/macros/prysma_nodiscard.h"

class IVisitor;
enum class NodeTypeGenerated;

class INode {
public:
    INode() = default;
    virtual ~INode() = default;

    INode(const INode&) = delete;
    auto operator=(const INode&) -> INode& = delete;
    INode(INode&&) = delete;
    auto operator=(INode&&) -> INode& = delete;

    virtual void accept(IVisitor* visitor) = 0;
    
    PRYSMA_NODISCARD virtual auto getGeneratedType() const -> NodeTypeGenerated = 0;
    PRYSMA_NODISCARD virtual auto getNodeId() const -> std::size_t = 0;

};
