//===-- multi_storage_registry.hpp ------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include "compiler/ast/registry/data/linear_table.hpp"
#include "compiler/ast/registry/data/smart_storage.hpp"
#include "compiler/ast/registry/data/storage_traits.hpp"
#include "compiler/macros/prysma_nodiscard.h"
#include <cstddef>
#include <tuple>
#include <utility>
#include <type_traits>
#include <iostream>

/***************************************************************************/

template<typename Table, typename HandleProvider, std::size_t N = 1 << 14>
class MultiStorageRegistry final {
protected:
    static constexpr std::size_t BytesPerStorage = N / size_of_v<Table>;

    template<typename Up> using RegistryStorageStrategy = SmartStorage<Up, BytesPerStorage / sizeof(Up)>;

public:
    explicit MultiStorageRegistry(HandleProvider provider = {}) // par copie
        : handleProvider_(provider), storage_{}
    {
        std::cout << "[NODE REGISTRY CTOR] this = " << this << "\n";
    }

    ~MultiStorageRegistry() noexcept { reset(); }

public:
    template<typename Tp>
    auto& resolve_storage() noexcept
    {
        using Result = std::decay_t<
            table_lookup_t<std::decay_t<Tp>, Table>
        >;

        static_assert(
            !std::is_same_v<Result, PRYSMA_SENTINEL>,
            "Unable to resolve the requested type from the LinearTable."
        );

        return std::get<RegistryStorageStrategy<Result>>(storage_);
    }

public:
    template<typename Tp>
    PRYSMA_NODISCARD const auto& get(const Tp* obj) const noexcept
    {
        const auto& storage = resolve_storage<Tp>();
        return storage.get(handleProvider_(obj));
    }

    template<typename Up, typename Tp>
    PRYSMA_NODISCARD const auto& get_for(const Tp* obj) const noexcept
    {
        const auto& storage = std::get<RegistryStorageStrategy<Up>>(storage_);
        return storage.get(handleProvider_(obj));
    }

public:
    // template<typename Tp>
    // PRYSMA_NODISCARD auto& get(const Tp* obj) noexcept
    // {
    //     std::cout << "calling get for -> " << typeid(Tp).name() << "\n";

    //     auto& storage = resolve_storage<Tp>();
    //     return storage.get(handleProvider_(obj));
    // }

    // template<typename Up, typename Tp>
    // PRYSMA_NODISCARD auto& get_for(const Tp* obj) noexcept
    // {
    //     std::cout << "calling get_for for -> " << typeid(Tp).name() << "\n";

    //     auto& storage = std::get<RegistryStorageStrategy<Up>>(storage_);
    //     return storage.get(handleProvider_(obj));
    // }

    template<typename Tp>
PRYSMA_NODISCARD auto& get(const Tp* obj) noexcept
{
    std::cout
        << "[GET] Tp = " << typeid(Tp).name()
        << " | obj = " << obj
        << std::endl;

    auto& storage = resolve_storage<Tp>();

    std::cout
        << "    -> resolved storage = " << &storage
        << std::endl;

    auto handle = handleProvider_(obj);

    std::cout
        << "    -> handle = " << handle
        << std::endl;

    auto& result = storage.get(handle);

    std::cout
        << "    -> SUCCESS get(handle)\n";

    return result;
}

template<typename Up, typename Tp>
PRYSMA_NODISCARD auto& get_for(const Tp* obj) noexcept
{
    std::cout
        << "[GET_FOR] Tp = " << typeid(Tp).name()
        << " | Up = " << typeid(Up).name()
        << " | obj = " << obj
        << std::endl;

    auto& storage = std::get<RegistryStorageStrategy<Up>>(storage_);

    std::cout
        << "    -> storage addr = " << &storage
        << std::endl;

    auto handle = handleProvider_(obj);

    std::cout
        << "    -> handle = " << handle
        << std::endl;

    auto& result = storage.get(handle);

    std::cout
        << "    -> SUCCESS get_for\n";

    return result;
}

public:
    template<typename Tp, typename... Types>
    auto& construct(const Tp* obj, Types&&... args)
    {
        auto& storage = resolve_storage<Tp>();
        return storage.emplace(handleProvider_(obj), std::forward<Types>(args)...);
    }

    template<typename Up, typename Tp, typename... Types>
    auto& construct_for(const Tp* obj, Types&&... args)
    {
        auto& storage = std::get<RegistryStorageStrategy<Up>>(storage_);
        return storage.emplace(handleProvider_(obj), std::forward<Types>(args)...);
    }

public:
    template<typename Tp, typename Up>
    auto& assign(const Tp* obj, Up&& arg)
    {
        auto& storage = resolve_storage<Tp>();
        return storage.insert(handleProvider_(obj), std::forward<Up>(arg));
    }

    template<typename Up, typename Tp>
    auto& assign_for(const Tp* obj, Up&& arg)
    {
        auto& storage = std::get<RegistryStorageStrategy<Up>>(storage_);
        return storage.insert(handleProvider_(obj), std::forward<Up>(arg));
    }

public:
    void reset() noexcept
    {
        std::apply([](auto&... storage) {
            (storage.reset(), ...);
        }, storage_);
    }

    template<typename Up>
    void reset_for() noexcept
    {
        auto& storage = std::get<RegistryStorageStrategy<Up>>(storage_);
        storage.reset();
    }

private:
    make_registry_storage_t<Table, RegistryStorageStrategy> storage_;
    HandleProvider handleProvider_;
};

/***************************************************************************/