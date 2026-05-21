//===-- registry_type.h -----------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef DD335087_6EDE_4036_872C_8BD586E26252
#define DD335087_6EDE_4036_872C_8BD586E26252

#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/ast/registry/registry_generic.h"
#include "interfaces/i_registry_type.h"
#include "compiler/lexer/token_type.h"
#include <llvm/IR/Type.h>
#include <string>

class RegistryType : public IRegistryType
{

public:
    RegistryType() = default;
    ~RegistryType() override = default;
    RegistryType(const RegistryType&) = delete;
    auto operator=(const RegistryType&) -> RegistryType& = delete;
    RegistryType(RegistryType&&) = delete;
    auto operator=(RegistryType&&) -> RegistryType& = delete;

protected:
    
    PRYSMA_NODISCARD auto generateErrorMessage(const TokenType& key) const -> std::string override {
        return RegistryGeneric::generateErrorMessage(key);
    }
};

#endif /* DD335087_6EDE_4036_872C_8BD586E26252 */
