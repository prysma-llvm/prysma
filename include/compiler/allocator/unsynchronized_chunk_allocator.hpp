//===-- unsynchronized_chunk_allocator.hpp ----------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include "compiler/memory/memory_resource.hpp"
#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/macros/prysma_unlikely.h"
#include "compiler/macros/prysma_likely.h"
#include "compiler/macros/prysma_maybe_unused.h"

#include <cstddef>
#include <iostream>
#include <new>
#include <stdexcept>
#include <utility>
#include <vector>

namespace prysma {

template<typename Tp>
struct unsynchronized_chunk_allocator {
private:
    static constexpr std::size_t default_chunk_capacity = 1024;
    static constexpr std::size_t default_initial_reserve = 10;
    static constexpr std::size_t default_chunk_refill = 5;
    static constexpr std::size_t default_chunk_refill_treshold = 1;

public:
    struct Chunk {
        std::byte* buffer_ = nullptr;
        std::size_t capacity_ = 0;
        std::size_t offset_ = 0;
    };

public:
    explicit unsynchronized_chunk_allocator(memory_resource* upstream, std::size_t reserve = default_initial_reserve)
        : ressource_{ upstream }
    {
        if (upstream == nullptr) PRYSMA_UNLIKELY_BRANCH
            throw std::invalid_argument("Upstream memory resource cannot be null");

        if (reserve == 0) PRYSMA_UNLIKELY_BRANCH
            throw std::invalid_argument("reserve must be >= 1");

        free_chunks_.reserve(std::size_t{ default_initial_reserve });

        for (std::size_t i = 0; i < reserve - 1; ++i) {
            void* raw = ressource_->allocate(sizeof(Tp) * default_chunk_capacity, alignof(Tp));
            free_chunks_.emplace_back(Chunk{ static_cast<std::byte*>(raw), default_chunk_capacity, std::size_t{0} });
        }

        void* raw = ressource_->allocate(sizeof(Tp) * default_chunk_capacity, alignof(Tp));
        active_chunk_ = Chunk{ static_cast<std::byte*>(raw), default_chunk_capacity, std::size_t{0} };
    }

    unsynchronized_chunk_allocator(const unsynchronized_chunk_allocator&) = delete;
    unsynchronized_chunk_allocator& operator=(const unsynchronized_chunk_allocator&) = delete;
    unsynchronized_chunk_allocator(unsynchronized_chunk_allocator&&) noexcept = default;
    unsynchronized_chunk_allocator& operator=(unsynchronized_chunk_allocator&&) noexcept = default;

    ~unsynchronized_chunk_allocator() noexcept {
        const std::size_t chunk_bytes = sizeof(Tp) * default_chunk_capacity;
        const std::size_t chunk_align = alignof(Tp);

        for (auto& chunk : free_chunks_) {
            if (chunk.buffer_ != nullptr) {
                ressource_->deallocate(static_cast<void*>(chunk.buffer_), chunk_bytes, chunk_align);
            }
        }

        if (active_chunk_.buffer_ != nullptr) PRYSMA_LIKELY_BRANCH {
            ressource_->deallocate(static_cast<void*>(active_chunk_.buffer_), chunk_bytes, chunk_align);
        }
    }

public:
    void refill(std::size_t count = default_chunk_refill) {
        for (std::size_t i = 0; i < count; ++i) {
            void* raw = ressource_->allocate(sizeof(Tp) * default_chunk_capacity, alignof(Tp));
            free_chunks_.emplace_back(Chunk{ static_cast<std::byte*>(raw), default_chunk_capacity, std::size_t{0} });
        }
    }

public:
    PRYSMA_NODISCARD Tp* allocate(std::size_t count) {
        if (free_chunks_.size() <= default_chunk_refill_treshold) PRYSMA_UNLIKELY_BRANCH {
            refill();
        }

        if (active_chunk_.buffer_ == nullptr) PRYSMA_UNLIKELY_BRANCH {
            if (free_chunks_.empty()) PRYSMA_UNLIKELY_BRANCH throw std::bad_alloc();

            active_chunk_ = std::move(free_chunks_.back());
            free_chunks_.pop_back();
        }

        const std::size_t requested = (sizeof(Tp) * count);

        std::size_t aligned = (active_chunk_.offset_ + (alignof(Tp) - 1)) & ~(alignof(Tp) - 1);
        std::size_t next_offset = aligned + requested;

        if (next_offset >= active_chunk_.capacity_) {
            if (free_chunks_.empty()) PRYSMA_UNLIKELY_BRANCH throw std::bad_alloc();

            active_chunk_ = std::move(free_chunks_.back());
            free_chunks_.pop_back();

            aligned = (active_chunk_.offset_ + (alignof(Tp) - 1)) & ~(alignof(Tp) - 1);
            next_offset = aligned + requested;

            if (next_offset > active_chunk_.capacity_) PRYSMA_UNLIKELY_BRANCH throw std::bad_alloc();
        }

        active_chunk_.offset_ = next_offset;
        return reinterpret_cast<Tp*>(active_chunk_.buffer_ + aligned);
    }

    PRYSMA_NODISCARD void* allocate_bytes(std::size_t bytes, std::size_t alignment) {
        if (free_chunks_.size() <= default_chunk_refill_treshold) PRYSMA_UNLIKELY_BRANCH {
            refill();
        }

        if (active_chunk_.buffer_ == nullptr) PRYSMA_UNLIKELY_BRANCH {
            if (free_chunks_.empty()) PRYSMA_UNLIKELY_BRANCH throw std::bad_alloc();

            active_chunk_ = std::move(free_chunks_.back());
            free_chunks_.pop_back();
        }

        std::size_t aligned = (active_chunk_.offset_ + (alignment - 1)) & ~(alignment - 1);
        std::size_t next_offset = aligned + bytes;

        if (next_offset >= active_chunk_.capacity_) {
            if (free_chunks_.empty()) PRYSMA_UNLIKELY_BRANCH throw std::bad_alloc();

            active_chunk_ = std::move(free_chunks_.back());
            free_chunks_.pop_back();

            aligned = (active_chunk_.offset_ + (alignment - 1)) & ~(alignment - 1);
            next_offset = aligned + bytes;

            if (next_offset > active_chunk_.capacity_) PRYSMA_UNLIKELY_BRANCH throw std::bad_alloc();
        }

        active_chunk_.offset_ = next_offset;
        return reinterpret_cast<void*>(active_chunk_.buffer_ + aligned);
    }

    template<typename Up>
    PRYSMA_NODISCARD Up* allocate_object(std::size_t count = 1) {
        void* const raw = allocate_bytes(sizeof(Up) * count, alignof(Up));
        return static_cast<Up*>(raw);
    }

public:
    void deallocate(
        PRYSMA_MAYBE_UNUSED Tp* const ptr,
        PRYSMA_MAYBE_UNUSED std::size_t count
    ) noexcept {
        // NO-OP: This chunk allocator does not support individual deallocations.
        // All memory is reclaimed at once via the global reset() method.
    }

    void deallocate_bytes(
        PRYSMA_MAYBE_UNUSED void* const ptr,
        PRYSMA_MAYBE_UNUSED std::size_t bytes,
        PRYSMA_MAYBE_UNUSED std::size_t alignment
    ) noexcept {
        // NO-OP: This chunk allocator does not support byte deallocations.
        // All memory is reclaimed at once via the global reset() method.
    }

    template<typename Up>
    void deallocate_object(
        PRYSMA_MAYBE_UNUSED Up* const ptr,
        PRYSMA_MAYBE_UNUSED std::size_t count = 1
    ) noexcept {
        // NO-OP: This chunk allocator does not support object deallocations.
        // All memory is reclaimed at once via the global reset() method.
    }

public:
    template<typename Up, typename... Types>
    void construct(Up* const ptr, Types&&... args) {
        if (!ptr) PRYSMA_UNLIKELY_BRANCH throw std::bad_alloc();

        ::new (const_cast<void*>(static_cast<const volatile void*>(ptr)))
            Up(std::forward<Types>(args)...);
    }

    template<typename Up>
    void destroy(Up* const ptr) noexcept {
        if (!ptr) PRYSMA_UNLIKELY_BRANCH throw std::bad_alloc();
        ptr->~Up();
    }

public:
    template<typename Up, typename... Types>
    PRYSMA_NODISCARD Up* new_object(Types&&... args) {
        Up* const ptr = allocate_object<Up>();
        construct(ptr, std::forward<Types>(args)...);

        return ptr;
    }

    template<typename Up>
    void delete_object(Up* const ptr) {
        destroy(ptr);
        deallocate_object<Up>();
    }

public:
    void info() {
        std::cout << "\n[UNSYNCHRONIZED CHUNK ALLOCATOR]\n";
        std::cout << "     Available chunks : " << free_chunks_.size() << "\n";
        std::cout << "     Active chunk    : " << (active_chunk_.buffer_ ? "valid" : "null") << "\n";
        std::cout << "        ptr      : " << static_cast<void*>(active_chunk_.buffer_) << "\n";
        std::cout << "        capacity : " << active_chunk_.capacity_ << "\n";
        std::cout << "        used     : " << active_chunk_.offset_ << "\n";

    }

private:
    std::vector<Chunk> free_chunks_;
    Chunk active_chunk_;

    memory_resource* ressource_;
};

}
