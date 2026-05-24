//===-- smart_storage.hpp ---------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include <array>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <iostream>
#include <variant>
#include <vector>
#include <llvm-18/llvm/ADT/DenseMap.h>
#include "compiler/macros/prysma_nodiscard.h"

template<typename Tp, std::size_t N>
class SmartStorage final {
protected:
    static constexpr std::uint16_t MaxStack = 1 << 14;

    using OnlyIfStackEligible = std::conditional_t<(N * sizeof(Tp) <= MaxStack),
                                                    std::byte[N * sizeof(Tp)],
                                                    std::monostate>;

public:
    SmartStorage() : is_constructed_{}, next_dense_index_{0} {
        if constexpr (is_heap_eligible_) {
            buffer_ptr_ = static_cast<std::byte*>( // heap alloc
                ::operator new(N * sizeof(Tp), std::align_val_t(alignof(Tp)))
            );
        }
        else { // stack alloc
            buffer_ptr_ = &stack_buffer_[0];
        }
    }

    ~SmartStorage() noexcept {
        reset();
        
        if constexpr (is_heap_eligible_) {
            ::operator delete(buffer_ptr_, std::align_val_t(alignof(Tp)));
        }
    }

protected:
    void throw_if_out_of_range(std::size_t index) const {
        if (index >= N) [[unlikely]] {
            throw std::out_of_range(
                "[PRYSMA::SmartStorage] index out of range: "
                + std::to_string(index) + " (valid range: 0.." + std::to_string(N - 1) + ")"
            );
        }
    }

    void throw_if_existing(std::size_t index) const {
        if (is_constructed_[index]) [[unlikely]] {
            throw std::runtime_error(
                "[PRYSMA::SmartStorage] construction conflict: slot already occupied at index "
                + std::to_string(index)
            );
        }
    }

    void throw_if_nonexistent(std::size_t index) const {
        if (!is_constructed_[index]) [[unlikely]] {
            throw std::runtime_error(
                "[PRYSMA::SmartStorage] access violation: no object constructed at index "
                + std::to_string(index)
            );
        }
    }

    // Pour convertir l'ID global en index interne compact
    std::size_t get_dense_index(std::size_t global_index) const {
        auto it = global_to_dense_.find(global_index);
        if (it == global_to_dense_.end()) [[unlikely]] {
            throw std::runtime_error(
                "[PRYSMA::SmartStorage] mapping error: no object mapped for global index "
                + std::to_string(global_index)
            );
        }
        return it->second;
    }

public:
    PRYSMA_NODISCARD Tp& get(std::size_t index) {
        std::size_t dense_index = get_dense_index(index);
        throw_if_out_of_range(dense_index); throw_if_nonexistent(dense_index);
        return *reinterpret_cast<Tp*>(buffer_ptr_ + dense_index * sizeof(Tp));
    }

    PRYSMA_NODISCARD const Tp& get(std::size_t index) const {
        std::size_t dense_index = get_dense_index(index);
        throw_if_out_of_range(dense_index); throw_if_nonexistent(dense_index);
        return *reinterpret_cast<const Tp*>(buffer_ptr_ + dense_index * sizeof(Tp));
    }

public:
    template<typename... Types>
    Tp& emplace(std::size_t index, Types&&... args)
        noexcept(std::is_nothrow_constructible_v<std::decay<Tp>, Types&&...>)
    {
        if (global_to_dense_.count(index)) [[unlikely]] {
            throw_if_existing(global_to_dense_[index]);
        }
        
        std::size_t dense_index;
        if (!free_indices_.empty()) {
            dense_index = free_indices_.back();
            free_indices_.pop_back();
        } else {
            dense_index = next_dense_index_++;
        }
        
        global_to_dense_[index] = dense_index;

        throw_if_out_of_range(dense_index);
        throw_if_existing(dense_index);

        Tp* ptr = reinterpret_cast<Tp*>(buffer_ptr_ + dense_index * sizeof(Tp));
        new (ptr) Tp(std::forward<Types>(args)...);

        is_constructed_[dense_index] = true;
        return *ptr;
    }

public:
    Tp& insert(std::size_t index, Tp&& obj)
        noexcept(
            std::is_nothrow_copy_constructible_v<Tp>
            && std::is_nothrow_assignable_v<Tp&, Tp&&>
        )
    {
        std::size_t dense_index;
        auto it = global_to_dense_.find(index);
        if (it != global_to_dense_.end()) {
            dense_index = it->second;
        } else {
            if (!free_indices_.empty()) {
                dense_index = free_indices_.back();
                free_indices_.pop_back();
            } else {
                dense_index = next_dense_index_++;
            }
            global_to_dense_[index] = dense_index;
        }

        throw_if_out_of_range(dense_index);

        Tp* ptr = reinterpret_cast<Tp*>(buffer_ptr_ + dense_index * sizeof(Tp));

        if (is_constructed_[dense_index]) {
            *ptr = obj;
        }
        else {
            new (ptr) Tp(obj);
            is_constructed_[dense_index] = true;
        }

        return *ptr;
    }

    Tp& insert(std::size_t index, Tp& obj)
        noexcept(
            std::is_nothrow_copy_constructible_v<Tp>
            && std::is_nothrow_assignable_v<Tp&, Tp&>
        )
    {
        std::size_t dense_index;
        auto it = global_to_dense_.find(index);
        if (it != global_to_dense_.end()) {
            dense_index = it->second;
        } else {
            if (!free_indices_.empty()) {
                dense_index = free_indices_.back();
                free_indices_.pop_back();
            } else {
                dense_index = next_dense_index_++;
            }
            global_to_dense_[index] = dense_index;
        }

        throw_if_out_of_range(dense_index);

        Tp* ptr = reinterpret_cast<Tp*>(buffer_ptr_ + dense_index * sizeof(Tp));

        if (is_constructed_[dense_index]) {
            *ptr = obj;
        }
        else {
            new (ptr) Tp(obj);
            is_constructed_[dense_index] = true;
        }

        return *ptr;
    }

public:
    void destroy(std::size_t index)
        noexcept(std::is_nothrow_destructible_v<Tp>)
    {
        std::size_t dense_index = get_dense_index(index);
        throw_if_out_of_range(dense_index);
        throw_if_nonexistent(dense_index);

        Tp* ptr = reinterpret_cast<Tp*>(buffer_ptr_ + dense_index * sizeof(Tp));
        ptr->~Tp();

        is_constructed_[dense_index] = false;
        
        free_indices_.push_back(dense_index);
        global_to_dense_.erase(index);
    }

public:
    void reset()
        noexcept(std::is_nothrow_destructible_v<Tp>)
    {
        for (std::size_t i = 0; i < next_dense_index_; ++i) { // Optimisé: on ne boucle que sur la zone utilisée
            if (is_constructed_[i]) {
                Tp* ptr = reinterpret_cast<Tp*>(buffer_ptr_ + i * sizeof(Tp));
                ptr->~Tp();
                is_constructed_[i] = false;
            }
        }

        std::memset(buffer_ptr_, 0, N * sizeof(Tp));
        global_to_dense_.clear();
        free_indices_.clear();
        next_dense_index_ = 0;
    }

public:
    PRYSMA_NODISCARD constexpr std::size_t capacity() const noexcept {
        return N;
    }

private:
    alignas(Tp) OnlyIfStackEligible stack_buffer_;
    std::byte* buffer_ptr_;

    static constexpr bool is_heap_eligible_ =
        std::is_same_v<OnlyIfStackEligible, std::monostate>;

    std::array<bool, N> is_constructed_;
    
    llvm::DenseMap<std::size_t, std::size_t> global_to_dense_;
    std::vector<std::size_t> free_indices_;
    std::size_t next_dense_index_;
};
