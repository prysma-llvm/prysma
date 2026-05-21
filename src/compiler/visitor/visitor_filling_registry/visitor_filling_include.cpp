//===-- visitor_filling_include.cpp -----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/ast/registry/context_gen_code.h"
#include "compiler/visitor/visitor_filling_registry/visitor_filling_registry.h"
#include "compiler/ast/utils/orchestrator_include/orchestrator_include.h" 
#include <filesystem>
#include <string>

void FillingVisitorRegistry::visiter(NodeInclude* nodeInclude)
{
    auto& nodeIncludeData = _contextGenCode->getNodeDataRegistry()->get(nodeInclude);

    std::filesystem::path parentFilePath(_contextGenCode->getCurrentFilePath());
    std::filesystem::path parentDirectory = parentFilePath.parent_path();
    std::filesystem::path absolutePath = parentDirectory / std::string(nodeIncludeData.getPath().value);

    _orchestrator->includeFile(absolutePath.string());
}

