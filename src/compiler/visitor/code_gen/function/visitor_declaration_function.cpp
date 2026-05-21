//===-- visitor_declaration_function.cpp ------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/llvm/gestion_function.h"
#include "compiler/visitor/code_gen/visitor_general_gen_code.h"

#include <memory>

void GeneralVisitorGenCode::visiter(NodeDeclarationFunction* nodeDeclarationFunction) 
{
    
    auto generator = FunctionDeclarationGenerator::create(_contextGenCode, nodeDeclarationFunction, this);
    generator->declareFunction();
}
