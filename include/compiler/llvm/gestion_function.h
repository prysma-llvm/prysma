//===-- gestion_function.h --------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef D2944365_C1DD_41F8_A211_BFF33402A958
#define D2944365_C1DD_41F8_A211_BFF33402A958

#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/ast/registry/context_gen_code.h"
#include "compiler/ast/registry/registry_function.h"
#include "compiler/visitor/interfaces/i_visitor.h"
#include <llvm-18/llvm/ADT/StringRef.h>
#include <llvm-18/llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <memory>
#include <vector>

class GeneralVisitorGenCode;
class NodeArgFunction;
class NodeDeclarationFunction;
class NodeCallFunction;

struct ArgumentsCodeGen {
    std::vector<llvm::Type*> argTypes;
    std::vector<NodeArgFunction*> arguments;
};

// Generation of the Declaration

class FunctionDeclarationGenerator {
private:
    ContextGenCode* _contextGenCode;
    NodeDeclarationFunction* _nodeDeclarationFunction;
    IVisitor* _visitorGeneralCodeGen;

    virtual auto createFunction() -> llvm::Function* = 0;
    virtual void handleConstructedArguments(llvm::Function* function, const ArgumentsCodeGen& args) = 0;

public:
    FunctionDeclarationGenerator(ContextGenCode* context, NodeDeclarationFunction* node, IVisitor* visitor);
    virtual ~FunctionDeclarationGenerator() = default;

    PRYSMA_NODISCARD auto getContextGenCode() const -> ContextGenCode* { return _contextGenCode; }
    PRYSMA_NODISCARD auto getNodeDeclarationFunction() const -> NodeDeclarationFunction* { return _nodeDeclarationFunction; }
    PRYSMA_NODISCARD auto getGeneralVisitorCodeGen() const -> IVisitor* { return _visitorGeneralCodeGen; }

    FunctionDeclarationGenerator(const FunctionDeclarationGenerator&) = delete;
    auto operator=(const FunctionDeclarationGenerator&) -> FunctionDeclarationGenerator& = delete;
    FunctionDeclarationGenerator(FunctionDeclarationGenerator&&) = delete;
    auto operator=(FunctionDeclarationGenerator&&) -> FunctionDeclarationGenerator& = delete;

    void declareFunction();
    
    static auto create(ContextGenCode* context, NodeDeclarationFunction* node, IVisitor* visitor) -> std::unique_ptr<FunctionDeclarationGenerator>;
};

class StandardFunctionDeclarationGenerator : public FunctionDeclarationGenerator {
private:
    auto createFunction() -> llvm::Function* override;
    void handleConstructedArguments(llvm::Function* function, const ArgumentsCodeGen& args) override;
public:
    using FunctionDeclarationGenerator::FunctionDeclarationGenerator;
};

class MethodFunctionDeclarationGenerator : public FunctionDeclarationGenerator {
private:
    auto createFunction() -> llvm::Function* override;
    void handleConstructedArguments(llvm::Function* function, const ArgumentsCodeGen& args) override;
public:
    using FunctionDeclarationGenerator::FunctionDeclarationGenerator;
};


// Generation of the Call
class FunctionCallGenerator {
private:
    ContextGenCode* _contextGenCode;
    IVisitor* _visitorGeneralCodeGen;

    virtual auto getLocalFunction(llvm::StringRef functionName) -> const SymbolFunctionLocal* = 0;

public:
    FunctionCallGenerator(ContextGenCode* context, IVisitor* visitor);
    virtual ~FunctionCallGenerator() = default;

    PRYSMA_NODISCARD auto getContextGenCode() const -> ContextGenCode* { return _contextGenCode; }
    PRYSMA_NODISCARD auto getGeneralVisitorCodeGen() const -> IVisitor* { return _visitorGeneralCodeGen; }

    FunctionCallGenerator(const FunctionCallGenerator&) = delete;
    auto operator=(const FunctionCallGenerator&) -> FunctionCallGenerator& = delete;
    FunctionCallGenerator(FunctionCallGenerator&&) = delete;
    auto operator=(FunctionCallGenerator&&) -> FunctionCallGenerator& = delete;

    void generateCallFunction(NodeCallFunction* node);
    static auto create(ContextGenCode* context, IVisitor* visitor) -> std::unique_ptr<FunctionCallGenerator>;
};

class StandardFunctionCallGenerator : public FunctionCallGenerator {
private:
    auto getLocalFunction(llvm::StringRef functionName) -> const SymbolFunctionLocal* override;
public:
    using FunctionCallGenerator::FunctionCallGenerator;
};

class MethodFunctionCallGenerator : public FunctionCallGenerator {
private:
    auto getLocalFunction(llvm::StringRef functionName) -> const SymbolFunctionLocal* override;
public:
    using FunctionCallGenerator::FunctionCallGenerator;
};

// Management of Native (Built-in) functions

class RegistryBuiltIns {
public:
    static auto isBuiltIn(llvm::StringRef name) -> bool;
    static void generateCall(llvm::StringRef name, NodeCallFunction* node, ContextGenCode* context, IVisitor* visitor);
};

#endif /* D2944365_C1DD_41F8_A211_BFF33402A958 */
