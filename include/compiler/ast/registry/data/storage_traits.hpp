//===-- storage_traits.hpp --------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include "compiler/ast/registry/data/linear_table.hpp"
#include <tuple>
#include <type_traits>

template<typename, template<typename...> typename>
struct make_registry_storage;

template<typename... Entries, template<typename...> typename StorageType>
struct make_registry_storage<LinearTable<Entries...>, StorageType> {
    using type = std::tuple<
        StorageType<typename entry_traits<Entries>::value>...
    >;
};

template<typename Table, template<typename...> typename StorageType>
using make_registry_storage_t = typename make_registry_storage<Table, StorageType>::type;