//===-- type_complex.h ------------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef C747FE4B_9A53_40BA_A4BE_19C00508C8C4
#define C747FE4B_9A53_40BA_A4BE_19C00508C8C4

#include "compiler/macros/prysma_nodiscard.h"
#include "i_type.h"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <string>

class TypeComplex : public IType {
private:
    std::string className;
public:
    explicit TypeComplex(const std::string& name);
    
    auto generateLLVMType(llvm::LLVMContext& context) -> llvm::Type* override;
    
    PRYSMA_NODISCARD auto isFloating() const -> bool override;
    PRYSMA_NODISCARD auto isBoolean() const -> bool override;
    PRYSMA_NODISCARD auto isString() const -> bool override;
    PRYSMA_NODISCARD auto isArray() const -> bool override;
    PRYSMA_NODISCARD auto isComplex() const -> bool override;

    PRYSMA_NODISCARD static auto classof(const IType* type) -> bool { return type->isComplex(); }
    
    auto getVTableType(llvm::LLVMContext& context) -> llvm::Type* override;
    
    PRYSMA_NODISCARD auto getClassName() const -> std::string;
};

#endif /* C747FE4B_9A53_40BA_A4BE_19C00508C8C4 */
