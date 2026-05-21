//===-- visitor_call_function.cpp -------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/visitor/code_gen/visitor_general_gen_code.h"
#include "compiler/llvm/gestion_function.h"

void GeneralVisitorGenCode::visiter(NodeCallFunction* nodeCallFunction)
{
    auto generator = FunctionCallGenerator::create(_contextGenCode, this);
    generator->generateCallFunction(nodeCallFunction);
}
