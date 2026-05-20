//===-- members_extractor_class.cpp -----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/visitor/extractors/members_extractor_class.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/registry/context_gen_code.h"
#include "compiler/visitor/visitor_base_generale.h"

MembersExtractorClass::MembersExtractorClass(ContextGenCode* contextGenCode)
    : VisitorBaseGenerale(contextGenCode) {}

MembersExtractorClass::~MembersExtractorClass()
= default;

void MembersExtractorClass::visiter(NodeDeclarationFunction* nodeClass) { 
    methods.push_back(nodeClass); 
}

void MembersExtractorClass::visiter(NodeDeclarationVariable* nodeClass) { 
    variables.push_back(nodeClass); 
}

void MembersExtractorClass::visiter(NodeClass* nodeClass) { 
    auto& nodeClassData = _contextGenCode->getNodeDataRegistry()->get(nodeClass);

    className = nodeClassData.getName().value; 
}
