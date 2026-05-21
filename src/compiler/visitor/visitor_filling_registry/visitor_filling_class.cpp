//===-- visitor_filling_class.cpp -------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/lexer/lexer.h"
#include "compiler/visitor/visitor_filling_registry/visitor_filling_registry.h"
#include "compiler/ast/registry/registry_class.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/ast/registry/registry_function.h"
#include "compiler/utils/prysma_cast.h"
#include "compiler/llvm/llvm_backend.h"

#include <llvm-18/llvm/IR/DerivedTypes.h>
#include <memory>
#include <string>
#include <utility>

// Pass 1: Register opaque class types
// We create an opaque StructType (without body) for each declared class.
// This allows resolving circular dependencies between classes
// during Pass 2 (struct body filling).

void FillingVisitorRegistry::visiter(NodeClass* nodeClass)
{
    auto& nodeClassData = _contextGenCode->getNodeDataRegistry()->get(nodeClass);

    // 1. Retrieve the class name from the declaration node
    const Token& classNameToken = nodeClassData.getName();
    std::string className = std::string(classNameToken.value);

    // 2. Create the "Opaque" type in LLVM
    llvm::StructType* opaqueTypeLLVM = llvm::StructType::create(
        _contextGenCode->getBackend()->getContext(),
        "Class_" + className
    );

    // 3. Create the "Class" structure instance containing metadata
    auto classInfo = std::make_unique<Class>();

    // 4. Link the opaque LLVM type to the structure
    classInfo->setStructType(opaqueTypeLLVM);

    // 5. Initialize inheritance
    const auto& heritage = nodeClassData.getInheritance();
    if (!heritage.empty()) {
        classInfo->setParentInheritance(heritage[0]);
    } else {
        classInfo->setParentInheritance(nullptr);
    }

    // 6. Prepare internal registries (empty for now, will be filled in Pass 2)
    classInfo->setRegistryVariable(new RegistryVariable());
    classInfo->setRegistryFunctionLocal(new RegistryFunctionLocal());

    // The VTable will be generated later during virtual method resolution
    classInfo->setVTable(nullptr);

    Class* classInfoPtr = classInfo.get();

    // 7. Register the class in the compiler's global registry
    _contextGenCode->getRegistryClass()->registerElement(className, std::move(classInfo));

    // 8. Visit the class body to fill its registries (methods, etc.)
    std::string previousClass = _contextGenCode->getCurrentClassName();
    _contextGenCode->setCurrentClassName(className);

    for (auto* member : nodeClassData.getMembers()) {
        if (prysma::isa<NodeDeclarationFunction>(member)) {
            member->accept(this);
        }
        else if (auto* declVar = prysma::dyn_cast<NodeDeclarationVariable>(member)) {
            auto& nodeDeclData = _contextGenCode->getNodeDataRegistry()->get(declVar);

            Token token;
            token.value = nodeDeclData.getName().value;
            classInfoPtr->getRegistryVariable()->registerVariable(token, Symbol(nullptr, nodeDeclData.getType()));
            
            if (nodeDeclData.getExpression() != nullptr) {
                classInfoPtr->getMemberInitializers()[std::string(nodeDeclData.getName().value)] = nodeDeclData.getExpression();
            }
        }
    }

    // Also visit builders if necessary:
    for (auto* builder : nodeClassData.getBuilder()) {
        builder->accept(this);
    }

    _contextGenCode->setCurrentClassName(previousClass);
}
