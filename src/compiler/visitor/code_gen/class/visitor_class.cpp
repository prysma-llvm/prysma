//===-- visitor_class.cpp ---------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/utils/prysma_cast.h"
#include "compiler/visitor/interfaces/i_visitor.h"
#include <string>

void GeneralVisitorGenCode::visiter(NodeClass* nodeClass)
{
    std::string previousClassName = _contextGenCode->getCurrentClassName();

    auto& nodeData = _contextGenCode->getNodeDataRegistry()->get(nodeClass);

    _contextGenCode->setCurrentClassName(std::string(nodeData.getName().value));

    for (auto* member : nodeData.getMembers()) {
        if (prysma::isa<NodeDeclarationFunction>(member)) {
            member->accept(this);
        }
    }

    for (auto* builder : nodeData.getBuilder()) {
        builder->accept(this);
    }

    _contextGenCode->setCurrentClassName(previousClassName);
}
