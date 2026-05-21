//===-- visitor_filling_registry.cpp ----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/utils/orchestrator_include/orchestrator_include.h"
#include "compiler/visitor/visitor_base_generale.h"
#include "compiler/visitor/visitor_filling_registry/visitor_filling_registry.h"

FillingVisitorRegistry::FillingVisitorRegistry(ContextGenCode* contextGenCode, OrchestratorInclude* orchestrator) 
    : VisitorBaseGenerale(contextGenCode), _orchestrator(orchestrator) {}

FillingVisitorRegistry::~FillingVisitorRegistry() = default;
