//===-- id_generator.hpp ----------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

// TODO: éventuellement changer d'emplacement car il n'est pas au bon endroit je crois

#pragma once

#include "compiler/ast/registry/types/i_type.h"
#include "compiler/macros/prysma_nodiscard.h"
#include <atomic>
#include <cstddef>

class IdGenerator final {
public:
    std::size_t next() noexcept {
        return current_.fetch_add(1, std::memory_order_relaxed);
    }

    PRYSMA_NODISCARD std::size_t current() const noexcept {
        return current_.load(std::memory_order_acquire);
    }

private:
    std::atomic<std::size_t> current_{0};
};