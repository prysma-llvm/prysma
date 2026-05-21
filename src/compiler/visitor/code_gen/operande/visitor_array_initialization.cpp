//===-- visitor_array_initialization.cpp ------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/macros/prysma_maybe_unused.h"

void GeneralVisitorGenCode::visiter(NodeArrayInitialization* nodeInitialization PRYSMA_MAYBE_UNUSED)
{
    // Cannot really initialize an array as a "temporary constant"
    // because arrays need to be allocated. This visitor will be called during
    // the evaluation of an expression containing [1, 2, 3, ...].
    // For now, return nullptr - the real handling must be done
    // in VisitorDeclarationVariable which knows the array type.
    
    _contextGenCode->setTemporaryValue(Symbol(nullptr, _contextGenCode->getTemporaryValue().getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
    _contextGenCode->setTemporaryValue(Symbol(_contextGenCode->getTemporaryValue().getAddress(), nullptr, _contextGenCode->getTemporaryValue().getPointedElementType()));
}
