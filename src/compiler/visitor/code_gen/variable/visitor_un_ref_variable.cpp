//===-- visitor_un_ref_variable.cpp -----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/llvm/gestion_variable.h"

void GeneralVisitorGenCode::visiter(NodeUnRefVariable* nodeUnRefVariable)
{
    VariableLoader loader(_contextGenCode);

    auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(nodeUnRefVariable);
    _contextGenCode->setTemporaryValue(loader.loadUnref(nodeData.getName().value));
}
