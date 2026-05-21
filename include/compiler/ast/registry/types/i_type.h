//===-- i_type.h ------------------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef BFF76A14_E9C5_4CB2_820B_010F0D0924A1
#define BFF76A14_E9C5_4CB2_820B_010F0D0924A1

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include "compiler/macros/prysma_nodiscard.h"

class IType {
public:
    IType() = default;
    IType(const IType&) = delete;
    auto operator=(const IType&) -> IType& = delete;
    IType(IType&&) = delete;
    auto operator=(IType&&) -> IType& = delete;
    virtual ~IType() = default;

    virtual auto generateLLVMType(llvm::LLVMContext& context) -> llvm::Type* = 0;
    
    // Utility methods to determine the characteristics of the type
    PRYSMA_NODISCARD virtual auto isFloating() const -> bool = 0;
    PRYSMA_NODISCARD virtual auto isBoolean() const -> bool = 0;
    PRYSMA_NODISCARD virtual auto isString() const -> bool = 0;
    PRYSMA_NODISCARD virtual auto isArray() const -> bool = 0;
    PRYSMA_NODISCARD virtual auto isComplex() const -> bool { return false; }
    virtual auto getVTableType(::llvm::LLVMContext& /*context*/) -> ::llvm::Type* { return nullptr; }
};

#endif /* BFF76A14_E9C5_4CB2_820B_010F0D0924A1 */
