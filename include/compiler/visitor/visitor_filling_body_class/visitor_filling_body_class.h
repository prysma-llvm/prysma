//===-- visitor_filling_body_class.h ----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef EC7E55EB_FEC0_4AFD_AEFD_1D5B3BCB89C0
#define EC7E55EB_FEC0_4AFD_AEFD_1D5B3BCB89C0

#include "compiler/ast/registry/context_gen_code.h"
#include "compiler/ast/registry/registry_class.h"
#include "compiler/visitor/visitor_base_generale.h"
#include <string>
#include <vector>

// Visitor that fills the body of a class (methods, vtable, etc.)
class FillingVisitorBodyClass : public VisitorBaseGenerale
{
private:
    using VisitorBaseGenerale::visiter;
    
    void buildVTable(Class* classInfo, const std::string& className, 
                     const std::vector<NodeDeclarationFunction*>& parentMethodList);
    
public:
    explicit FillingVisitorBodyClass(ContextGenCode* contextGenCode);
    ~FillingVisitorBodyClass() override;

    FillingVisitorBodyClass(const FillingVisitorBodyClass&) = delete;
    auto operator=(const FillingVisitorBodyClass&) -> FillingVisitorBodyClass& = delete;
    FillingVisitorBodyClass(FillingVisitorBodyClass&&) = delete;
    auto operator=(FillingVisitorBodyClass&&) -> FillingVisitorBodyClass& = delete;

    void visiter(NodeClass* nodeClass) override;
};

#endif /* EC7E55EB_FEC0_4AFD_AEFD_1D5B3BCB89C0 */
