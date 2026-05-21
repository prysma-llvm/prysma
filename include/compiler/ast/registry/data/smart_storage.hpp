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
#include "compiler/macros/prysma_nodiscard.h"

template<typename Tp, std::size_t N>
class SmartStorage final {
protected:
    static constexpr std::uint16_t MaxStack = 1 << 14;

    using OnlyIfStackEligible = std::conditional_t<(N * sizeof(Tp) <= MaxStack),
                                                    std::byte[N * sizeof(Tp)],
                                                    std::monostate>;

public:
    SmartStorage() : is_constructed_{} {
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

public:
    // PRYSMA_NODISCARD Tp& get(std::size_t index) {
    //     throw_if_out_of_range(index); throw_if_nonexistent(index);
    //     return *reinterpret_cast<Tp*>(buffer_ptr_ + index * sizeof(Tp));
    // }

    PRYSMA_NODISCARD Tp& get(std::size_t index)
{
    std::cout
        << "[SmartStorage::get] index = " << index
        << " | buffer_ptr = " << static_cast<void*>(buffer_ptr_)
        << std::endl;

                std::cout << " | type -> " << typeid(Tp).name() << "\n";

    try {
        throw_if_out_of_range(index);
        std::cout << "    -> in range OK\n";

        throw_if_nonexistent(index);
        std::cout << "    -> exists OK\n";
    }
    catch (const std::exception& e) {
        std::cout
            << "    -> THROW in validation: " << e.what()
            << std::endl;
        throw;
    }

    auto* addr = reinterpret_cast<Tp*>(buffer_ptr_ + index * sizeof(Tp));

    std::cout
        << "    -> computed addr = " << static_cast<void*>(addr)
        << " (offset = " << (index * sizeof(Tp)) << ")"
        << std::endl;

    return *addr;
}

    // PRYSMA_NODISCARD const Tp& get(std::size_t index) const {
    //     throw_if_out_of_range(index); throw_if_nonexistent(index);
    //     return *reinterpret_cast<const Tp*>(buffer_ptr_ + index * sizeof(Tp));

    // }

    PRYSMA_NODISCARD const Tp& get(std::size_t index) const
{
    std::cout
        << "[SmartStorage::get const] index = " << index
        << " | buffer_ptr = " << static_cast<const void*>(buffer_ptr_)
        << std::endl;

        std::cout << " | type -> " << typeid(Tp).name() << "\n";
    throw_if_out_of_range(index);
    throw_if_nonexistent(index);

    auto* addr = reinterpret_cast<const Tp*>(
        buffer_ptr_ + index * sizeof(Tp)
    );

    std::cout
        << "    -> computed addr = " << static_cast<const void*>(addr)
        << std::endl;

    return *addr;
}

public:
    template<typename... Types>
    Tp& emplace(std::size_t index, Types&&... args)
        noexcept(std::is_nothrow_constructible_v<std::decay<Tp>, Types&&...>)
    {
        std::cout << "buffer capacity: " << N * sizeof(Tp) << "\n";
        throw_if_out_of_range(index);
        throw_if_existing(index);

        Tp* ptr = reinterpret_cast<Tp*>(buffer_ptr_ + index * sizeof(Tp));
        new (ptr) Tp(std::forward<Types>(args)...);

            std::cout
        << "[SMART_STORAGE] construct index = " << index
        << std::endl;

                std::cout << " | type -> " << typeid(Tp).name() << "\n";

        is_constructed_[index] = true;
        return *ptr;
    }

public:
    Tp& insert(std::size_t index, Tp&& obj)
        noexcept(
            std::is_nothrow_copy_constructible_v<Tp>
            && std::is_nothrow_assignable_v<Tp&, Tp&&>
        )
    {
        throw_if_out_of_range(index);

        Tp* ptr = reinterpret_cast<Tp*>(buffer_ptr_ + index * sizeof(Tp));

        if (is_constructed_[index]) {
            *ptr = obj;
        }
        else {
            new (ptr) Tp(obj);
            is_constructed_[index] = true;
        }

        return *ptr;
    }

    Tp& insert(std::size_t index, Tp& obj)
        noexcept(
            std::is_nothrow_copy_constructible_v<Tp>
            && std::is_nothrow_assignable_v<Tp&, Tp&>
        )
    {
        throw_if_out_of_range(index);

        Tp* ptr = reinterpret_cast<Tp*>(buffer_ptr_ + index * sizeof(Tp));

        if (is_constructed_[index]) {
            *ptr = obj;
        }
        else {
            new (ptr) Tp(obj);
            is_constructed_[index] = true;
        }

        return *ptr;
    }

public:
    void destroy(std::size_t index)
        noexcept(std::is_nothrow_destructible_v<Tp>)
    {
        throw_if_out_of_range(index);
        throw_if_nonexistent(index);

        Tp* ptr = reinterpret_cast<Tp*>(buffer_ptr_ + index * sizeof(Tp));
        ptr->~Tp();

        is_constructed_[index] = false;
    }

public:
    void reset()
        noexcept(std::is_nothrow_destructible_v<Tp>)
    {
        for (std::size_t i = 0; i < N; ++i) {
            if (is_constructed_[i]) {
                Tp* ptr = reinterpret_cast<Tp*>(buffer_ptr_ + i * sizeof(Tp));
                ptr->~Tp();
                is_constructed_[i] = false;
            }
        }

        std::memset(buffer_ptr_, 0, N * sizeof(Tp));
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
};