//===-- handle_providers.hpp ------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/macros/prysma_nodiscard.h"
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include "compiler/macros/prysma_unlikely.h"

struct DefaultHandleProvider {
    template<typename Tp>
    PRYSMA_NODISCARD constexpr std::size_t operator()(const Tp* obj) {
        return obj->id;
    }
};

struct NodeHandleProvider {
    PRYSMA_NODISCARD constexpr std::size_t operator()(const INode* node) {
        if (node == nullptr) PRYSMA_UNLIKELY_BRANCH {
            throw std::invalid_argument("NodeHandleProvider received nullptr");
        }

        return node->getNodeId();
    }
};