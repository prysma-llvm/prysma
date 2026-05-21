//===-- registry_symbole.h --------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/ast/registry/registry_generic.h"
#include "compiler/ast/registry/interfaces/i_registry_symbole.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include <functional>
#include <set>
#include <string>
#include <utility>


class IExpression;

class RegistrySymbol : public RegistryGeneric<TokenType, std::function<IExpression*(Token)>>,
                      public IRegistrySymbol {
public:
    RegistrySymbol() = default;
    RegistrySymbol(const RegistrySymbol&) = delete;
    auto operator=(const RegistrySymbol&) -> RegistrySymbol& = delete;
    RegistrySymbol(RegistrySymbol&&) = delete;
    auto operator=(RegistrySymbol&&) -> RegistrySymbol& = delete;
    ~RegistrySymbol() override = default;

    void registerSymbol(
        TokenType symbol, 
        std::function<IExpression*(Token)> provider) override {
        RegistryGeneric::registerElement(symbol, std::move(provider));
    }

    auto getNode(Token token) -> IExpression* override {
        std::function<IExpression*(Token)> provider = RegistryGeneric::get(token.type);
        return provider(token);
    }

    PRYSMA_NODISCARD auto isOperator(TokenType symbol) const -> bool override {
        return exists(symbol);
    }

    PRYSMA_NODISCARD auto getSymbols() const -> std::set<TokenType> override {
        return getKeys();
    }

protected:
    using RegistryGeneric<TokenType, std::function<IExpression*(Token)>>::generateErrorMessage;
    PRYSMA_NODISCARD auto generateErrorMessage(const TokenType& key) const -> std::string override {
        return RegistryGeneric::generateErrorMessage(key);
    }
};
