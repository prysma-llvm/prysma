//===-- type_array.cpp ------------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/registry/types/type_array.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/data/node_data_registry.hpp"
#include "compiler/ast/registry/types/i_type.h"
#include "compiler/utils/prysma_cast.h"
#include <cstdint>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <stdexcept>
#include <string>

// Obligation de passer le registre ici. Il s'agirait de trouver une solution 
// ce n'est pas très optimal comme design. Patch temporaire.
TypeArray::TypeArray(IType* childType, INode* size, NodeDataRegistry* nodeDataRegistry)
    : _childType(childType), _size(size), _nodeDataRegistry(nodeDataRegistry)
{}

auto TypeArray::generateLLVMType(llvm::LLVMContext& context) -> llvm::Type*
{
    llvm::Type* elementType = _childType->generateLLVMType(context);

    // If the size is nullptr (dynamic size array), return a pointer
    // because as a parameter or dynamic allocation, it represents an address.
    if (_size == nullptr) {
        return llvm::PointerType::getUnqual(context);
    }

    NodeLiteral* literal = nullptr;

    if (prysma::isa<NodeLiteral>(_size)) {
        literal = prysma::cast<NodeLiteral>(_size);
    }
    
    if (literal == nullptr) {
        throw std::runtime_error("Error: the array size must be an integer literal");
    }

    std::cout << "PASSED NODELITERAL NULLPTR CHECK\n";

    auto& nodeLiteralData = _nodeDataRegistry->get(literal);

    auto size = static_cast<uint64_t>(std::stoull(std::string(nodeLiteralData.getToken().value)));
    return llvm::ArrayType::get(elementType, size);
}

auto TypeArray::isFloating() const -> bool
{
    return false;
}

auto TypeArray::isBoolean() const -> bool
{
    return false;
}

auto TypeArray::isString() const -> bool
{
    return _childType->isString();
}
