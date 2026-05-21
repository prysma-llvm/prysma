//===-- visitor_filling_registry.h ------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef E5CE13D3_6823_4DDE_BB20_5311A0E477B6
#define E5CE13D3_6823_4DDE_BB20_5311A0E477B6
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/registry/context_gen_code.h"
#include "compiler/visitor/visitor_base_generale.h"

// This visitor is intended to fill the function and variable registries before code generation.
// It traverses the abstract syntax tree to fill the registries with nullptr values for functions and variables.
// This allows a first pass to populate the registries.

class OrchestratorInclude;

class FillingVisitorRegistry : public VisitorBaseGenerale
{
private:
    OrchestratorInclude* _orchestrator;
    
public:
    using VisitorBaseGenerale::visiter;
    FillingVisitorRegistry(ContextGenCode* contextGenCode, OrchestratorInclude* orchestrator);
    ~FillingVisitorRegistry() override;

    FillingVisitorRegistry(const FillingVisitorRegistry&) = delete;
    auto operator=(const FillingVisitorRegistry&) -> FillingVisitorRegistry& = delete;
    FillingVisitorRegistry(FillingVisitorRegistry&&) = delete;
    auto operator=(FillingVisitorRegistry&&) -> FillingVisitorRegistry& = delete;
    void visiter(NodeDeclarationVariable* nodeDeclarationVariable) override;
    void visiter(NodeDeclarationFunction* nodeDeclarationFunction) override;
    void visiter(NodeInclude* nodeInclude) override;
    void visiter(NodeClass* nodeClass) override;
};

#endif /* E5CE13D3_6823_4DDE_BB20_5311A0E477B6 */
