//===-- sharded_string_interner.hpp -----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include "compiler/allocator/unsynchronized_chunk_allocator.hpp"
#include "compiler/memory/memory_resource.hpp"
#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/macros/prysma_unlikely.h"
#include "compiler/interning/string_interner.hpp"

#include <array>
#include <cstddef>
#include <functional>
#include <string_view>


// INFO: how to use this class ?

//  monotonic_atomic_buffer charpool(1 << 18);
//  sharded_string_interner<10> interner{ &charpool };

//  auto strptr = interner.intern("this was made by unrays");

template<std::size_t N>
struct sharded_string_interner {
public:
    using Shard = StringInterner;

public:
    explicit sharded_string_interner(prysma::memory_resource* upstream) {
        for (std::size_t i = 0; i < N; ++i) {
            shards_[i] = ::new Shard(prysma::unsynchronized_chunk_allocator<char>(upstream));
        }
    }

    ~sharded_string_interner() noexcept{
        for (auto* shard : shards_) {
            if (shard == nullptr) PRYSMA_UNLIKELY_BRANCH continue;
            delete shard;
        }
    }

public:
    PRYSMA_NODISCARD const char* const intern(std::string_view sv) {
        std::hash<std::string_view> h; std::size_t index = h(sv) % N;
        return shards_[index]->intern(sv);
    }

private:
    std::array<Shard*, N> shards_;
};