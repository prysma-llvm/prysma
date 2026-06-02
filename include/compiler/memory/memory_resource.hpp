//===-- memory_ressource.hpp ------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cstddef>
#include <stdexcept>
#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/macros/prysma_unlikely.h"

namespace prysma {

class memory_resource {
public:
    virtual ~memory_resource() = default;

public:
    PRYSMA_NODISCARD void* allocate(std::size_t bytes, std::size_t alignment) noexcept {
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) PRYSMA_UNLIKELY_BRANCH
            throw std::invalid_argument("alignment must be a power of two");
        return do_allocate(bytes, alignment);
    }

    void deallocate(void* p, std::size_t bytes, std::size_t alignment) noexcept {
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) PRYSMA_UNLIKELY_BRANCH
            throw std::invalid_argument("alignment must be a power of two");
        do_deallocate(p, bytes, alignment);
    }

    bool is_equal(const memory_resource& other) const noexcept {
        return do_is_equal(other);
    }

protected:
    virtual void* do_allocate(std::size_t bytes, std::size_t alignment) = 0;
    virtual void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) = 0;

    virtual bool do_is_equal(const memory_resource& other) const noexcept = 0;
};

}