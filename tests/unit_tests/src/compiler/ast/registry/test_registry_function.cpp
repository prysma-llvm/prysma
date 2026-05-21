//===-- test_registry_function.cpp ------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "catch.hpp"
#include "compiler/ast/registry/registry_function.h"
#include <string>
#include <memory>

using namespace std;

// Tests du registry de functions (globale et locale)

TEST_CASE("Enregistryr et get une function globale", "[RegistryFunction]") {
    RegistryFunctionGlobal registry;

    auto symbole = make_unique<SymbolFunctionGlobal>();
    symbole->returnType = nullptr;

    registry.registerElement("maFunction", std::move(symbole));

    CHECK(registry.exists("maFunction"));

    const auto& recupere = registry.get("maFunction");
    auto* symboleCast = dynamic_cast<SymbolFunctionGlobal*>(recupere.get());
    REQUIRE(symboleCast != nullptr);
}

TEST_CASE("Recuperer une function inexistante lance une exception", "[RegistryFunction]") {
    RegistryFunctionGlobal registry;

    CHECK_FALSE(registry.exists("fantome"));
    CHECK_THROWS_AS(registry.get("fantome"), std::invalid_argument);
}

TEST_CASE("Enregistryr plusieurs functions et verifier les cles", "[RegistryFunction]") {
    RegistryFunctionGlobal registry;

    registry.registerElement("fn_a", make_unique<SymbolFunctionGlobal>());
    registry.registerElement("fn_b", make_unique<SymbolFunctionGlobal>());
    registry.registerElement("fn_c", make_unique<SymbolFunctionGlobal>());

    auto cles = registry.getKeys();
    CHECK(cles.size() == 3);
    CHECK(cles.count("fn_a") == 1);
    CHECK(cles.count("fn_b") == 1);
    CHECK(cles.count("fn_c") == 1);
}

TEST_CASE("Ecraser une function existante dans le registry", "[RegistryFunction]") {
    RegistryFunctionLocal registry;

    auto premier = make_unique<SymbolFunctionLocal>();
    premier->function = nullptr;
    registry.registerElement("update", std::move(premier));

    auto second = make_unique<SymbolFunctionLocal>();
    second->function = reinterpret_cast<llvm::Function*>(0xDEAD);
    registry.registerElement("update", std::move(second));

    const auto& recupere = registry.get("update");
    auto* symboleCast = dynamic_cast<SymbolFunctionLocal*>(recupere.get());
    REQUIRE(symboleCast != nullptr);
    CHECK(symboleCast->function == reinterpret_cast<llvm::Function*>(0xDEAD));
}

TEST_CASE("Registry local functionne independamment du global", "[RegistryFunction]") {
    RegistryFunctionGlobal registryGlobal;
    RegistryFunctionLocal registryLocal;

    registryGlobal.registerElement("shared", make_unique<SymbolFunctionGlobal>());
    registryLocal.registerElement("shared", make_unique<SymbolFunctionLocal>());

    CHECK(registryGlobal.exists("shared"));
    CHECK(registryLocal.exists("shared"));

    const auto& globale = registryGlobal.get("shared");
    const auto& locale = registryLocal.get("shared");

    CHECK(dynamic_cast<SymbolFunctionGlobal*>(globale.get()) != nullptr);
    CHECK(dynamic_cast<SymbolFunctionLocal*>(locale.get()) != nullptr);
}

// cas non functionnel sad test 
TEST_CASE("RegistryFunction - Enregistryr null et get ne plante pas", "[RegistryFunction][SadTest]") {
    RegistryFunctionLocal registry;

    registry.registerElement("fn1", make_unique<SymbolFunctionLocal>());
    registry.registerElement("fn2", make_unique<SymbolFunctionLocal>());

    CHECK(registry.exists("fn1"));
    CHECK(registry.exists("fn2"));
    CHECK(registry.getKeys().size() == 2);

    auto fnUpdate = make_unique<SymbolFunctionLocal>();
    fnUpdate->function = reinterpret_cast<llvm::Function*>(0xABCD);
    registry.registerElement("fn1", std::move(fnUpdate));

    const auto& recupere = registry.get("fn1");
    auto* symbole = dynamic_cast<SymbolFunctionLocal*>(recupere.get());
    REQUIRE(symbole != nullptr);
    CHECK(symbole->function == reinterpret_cast<llvm::Function*>(0xABCD));

    CHECK(registry.getKeys().size() == 2);
    CHECK_THROWS_AS(registry.get("fn_inexistante"), std::invalid_argument);
}
