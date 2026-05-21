//===-- arg_extractor_function.cpp ------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/visitor/extractors/arg_extractor_function.h"
#include "compiler/ast/registry/context_gen_code.h"
#include "compiler/visitor/visitor_base_generale.h"

ArgExtractorFunction::ArgExtractorFunction(ContextGenCode* contextGenCode)
    : VisitorBaseGenerale(contextGenCode)
{} // inutile pour l'instant mais c'est pour la cohérence et l'extensibilité

ArgExtractorFunction::~ArgExtractorFunction()
= default;

void ArgExtractorFunction::visiter(NodeArgFunction* nodeArgFunction) {
    arg = nodeArgFunction;
}
