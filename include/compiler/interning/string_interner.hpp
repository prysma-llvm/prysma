//===-- string_interner.hpp -------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include "compiler/allocator/unsynchronized_chunk_allocator.hpp"
#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/macros/prysma_unlikely.h"

#include <cstring>
#include <mutex>
#include <new>
#include <shared_mutex>
#include <string_view>
#include <unordered_set>
#include <iostream>

class alignas(std::hardware_destructive_interference_size) StringInterner {
public:
    explicit StringInterner(prysma::unsynchronized_chunk_allocator<char>&& upstream)
        : ressource_{ std::move(upstream) } {}

    ~StringInterner() = default;

public:
    const char* intern(std::string_view sv) {
        std::cout << "STATE BEFORE RESOLVING: "; ressource_.info();

        {
            std::shared_lock<std::shared_mutex> rlock(shared_mutex_);

            if (auto it = table_.find(sv); it != table_.end()) {
                std::cout << "'" << sv << "' is already allocated in the system";
                return it->data();
            }
        }

        std::cout << "'" << sv << "'" << " is not allocated in the system yet, proceeding to allocation...\n";

        {
            std::unique_lock<std::shared_mutex> wlock(shared_mutex_);

            if (auto it = table_.find(sv); it != table_.end()) PRYSMA_UNLIKELY_BRANCH {
                return it->data();
            }

            char* cptr = static_cast<char*>(ressource_.allocate_bytes(sv.size() + 1, 1));

            std::memcpy(cptr, sv.data(), sv.size());
            cptr[sv.size()] = '\0';

            table_.emplace(std::string_view(cptr));

            std::cout << "\n\nSTATE AFTER RESOLVING: "; ressource_.info();

            return static_cast<const char*>(cptr);
        }
    }

private:
    std::unordered_set<std::string_view> table_;
    prysma::unsynchronized_chunk_allocator<char> ressource_;

    std::shared_mutex shared_mutex_;
};