//===-- visitor_general_gen_code.h ------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef FDDB5968_6DEF_49B0_AD11_8791D523278A
#define FDDB5968_6DEF_49B0_AD11_8791D523278A

#include <llvm-18/llvm/IR/Value.h>
#include <string>
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_gen_code.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/visitor/interfaces/i_visitor.h"
#include "compiler/visitor/visitor_base_generale.h"


class RegistryVariable; 
class LlvmBackend; 
class OrchestratorInclude;
class NodeInstruction;
class NodeAssignmentVariable;
class NodeDeclarationVariable;
class NodeCallFunction;
class NodeReturn;
class NodeArgFunction;
class NodeDeclarationFunction;
class NodeLiteral;
class NodeUnRefVariable;
class NodeRefVariable;
class NodeIf;
class NodeElse;
class NodeWhile;
class NodeOperation;
class NodeNegation;
class NodeArrayInitialization;
class NodeAssignmentArray;
class NodeReadingArray;
class NodeNew;
class NodeDelete;
class NodeCallObject;

class GeneralVisitorGenCode : public VisitorBaseGenerale
{
private:
    OrchestratorInclude* _orchestratorInclude;

public:
    // Prevent accidental copy and move of this visitor
    GeneralVisitorGenCode(const GeneralVisitorGenCode&) = delete;
    auto operator=(const GeneralVisitorGenCode&) -> GeneralVisitorGenCode& = delete;
    GeneralVisitorGenCode(GeneralVisitorGenCode&&) = delete;
    auto operator=(GeneralVisitorGenCode&&) -> GeneralVisitorGenCode& = delete;

    using VisitorBaseGenerale::visiter;
    void visiter(NodeAssignmentVariable* nodeAssignmentVariable) override;
    void visiter(NodeDeclarationVariable* nodeDeclarationVariable) override;
    void visiter(NodeCallFunction* nodeCallFunction) override;
    void visiter(NodeReturn* nodeReturn) override;
    void visiter(NodeArgFunction* nodeArgFunction) override;
    void visiter(NodeDeclarationFunction* nodeDeclarationFunction) override;
    void visiter(NodeOperation* nodeOperation) override;
    void visiter(NodeLiteral* nodeLiteral) override;
    void visiter(NodeInstruction* instruction) override;
    void visiter(NodeUnRefVariable* nodeUnRefVariable) override;
    void visiter(NodeRefVariable* nodeRefVariable) override;
    void visiter(NodeIf* nodeIf) override;
    void visiter(NodeWhile* nodeWhile) override;
    void visiter(NodeNegation* nodeNegation) override;
    void visiter(NodeArrayInitialization* nodeArrayInit) override;
    void visiter(NodeAssignmentArray* nodeAssignmentArray) override;
    void visiter(NodeReadingArray* nodeReadingArray) override;
    void visiter(NodeNew* nodeNew) override;
    void visiter(NodeDelete* nodeDelete) override;
    void visiter(NodeClass* nodeClass) override;
    void visiter(NodeCallObject* nodeCallObject) override;
    void visiter(NodeAccesAttribute* nodeAccesAttribute) override;
    GeneralVisitorGenCode(ContextGenCode* contextGenCode, OrchestratorInclude* orchestratorInclude);
    ~GeneralVisitorGenCode() override;
    void traverseChild(NodeInstruction* node);
    
    auto evaluateExpression(INode* expression) -> Symbol;
    
    static auto getClassNameFromSymbol(const Symbol& objectSymbol) -> std::string;
};

#endif /* FDDB5968_6DEF_49B0_AD11_8791D523278A */
