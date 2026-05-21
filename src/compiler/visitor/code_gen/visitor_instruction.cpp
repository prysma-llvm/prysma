//===-- visitor_instruction.cpp ---------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"

void GeneralVisitorGenCode::visiter(NodeInstruction* instruction)
{
    if (instruction != nullptr) {
        auto& component = _contextGenCode->getNodeDataRegistry()->get(instruction);

        for (const auto& child : component.getChildren()) {
            if (child != nullptr) {
                child->accept(this);
            }
        }
    }
}
