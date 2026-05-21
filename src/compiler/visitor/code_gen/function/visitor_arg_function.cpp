//===-- visitor_arg_function.cpp --------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/macros/prysma_maybe_unused.h"

void GeneralVisitorGenCode::visiter(PRYSMA_MAYBE_UNUSED NodeArgFunction* nodeArgFunction) 
{
    _contextGenCode->setTemporaryValue(Symbol(nullptr, _contextGenCode->getTemporaryValue().getType(), _contextGenCode->getTemporaryValue().getPointedElementType()));
}
