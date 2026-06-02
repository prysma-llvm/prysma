//===-- monotonic_atomic_buffer.h -------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include "memory_resource.hpp"
#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/macros/prysma_unlikely.h"
#include <atomic>
#include <cstddef>

namespace prysma {

struct monotonic_atomic_buffer : public memory_resource {
public:
    explicit monotonic_atomic_buffer(std::size_t p_capacity)
        : capacity_(p_capacity)
        , offset_(0)
        , buffer_(new std::byte[p_capacity])
    {}

    monotonic_atomic_buffer(const monotonic_atomic_buffer&) = delete;
    monotonic_atomic_buffer& operator=(const monotonic_atomic_buffer&) = delete;

    ~monotonic_atomic_buffer() noexcept override {
        delete[] buffer_;
    }

protected:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        std::size_t current = offset_.load(std::memory_order_relaxed);

        while (true) {
            std::size_t aligned = (current + (alignment - 1)) & ~(alignment - 1);
            std::size_t next = aligned + bytes;

            if (next > capacity_)
                return nullptr;

            if (offset_.compare_exchange_weak(
                current,
                next,
                std::memory_order_relaxed)) {
                return buffer_ + aligned;
            }
        }
    }

    void do_deallocate(void*, std::size_t, std::size_t) override {}

    bool do_is_equal(const memory_resource& other) const noexcept override {
        return this == &other;
    }

public:
    PRYSMA_NODISCARD constexpr std::size_t capacity() const noexcept { return capacity_; }
    PRYSMA_NODISCARD std::size_t size() const noexcept { return offset_.load(std::memory_order_relaxed); }

private:
    std::byte* buffer_;
    std::atomic<std::size_t> offset_;
    std::size_t capacity_;
};

}
