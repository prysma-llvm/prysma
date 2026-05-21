//===-- gestion_function.cpp ------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/llvm/gestion_function.h"
#include "compiler/ast/registry/context_gen_code.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/ast/registry/registry_function.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include "compiler/visitor/interfaces/i_visitor.h"
#include "compiler/visitor/visitor_base_generale.h"
#include "compiler/visitor/extractors/arg_extractor_function.h"
#include "compiler/utils/prysma_cast.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <llvm-18/llvm/ADT/ArrayRef.h>
#include <llvm-18/llvm/ADT/StringRef.h>
#include <llvm-18/llvm/IR/Function.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


// Generation of the Declaration

std::unique_ptr<FunctionDeclarationGenerator> FunctionDeclarationGenerator::create(ContextGenCode* context, NodeDeclarationFunction* node, IVisitor* visitor)
{
    if (!context->getCurrentClassName().empty()) {
        return std::make_unique<MethodFunctionDeclarationGenerator>(context, node, visitor);
    } 
    return std::make_unique<StandardFunctionDeclarationGenerator>(context, node, visitor);
}

FunctionDeclarationGenerator::FunctionDeclarationGenerator(ContextGenCode* contextGenCode, NodeDeclarationFunction* nodeDeclarationFunction, IVisitor* visitorGeneralCodeGen) 
:   _contextGenCode(contextGenCode),
    _nodeDeclarationFunction(nodeDeclarationFunction),
    _visitorGeneralCodeGen(visitorGeneralCodeGen)
{
}

auto StandardFunctionDeclarationGenerator::createFunction() -> llvm::Function*
{
    auto& nodeData = getContextGenCode()->getNodeDataRegistry()->get(getNodeDeclarationFunction());

    llvm::StringRef functionName = nodeData.getName().value;

    const auto& symbolPtr = getContextGenCode()->getRegistryFunctionLocal()->get(functionName);
    if (!prysma::isa<SymbolFunctionLocal>(symbolPtr.get())) {
        throw std::runtime_error("Error: Expected SymbolFunctionLocal");
    }
    const auto* symbol = prysma::cast<const SymbolFunctionLocal>(symbolPtr.get());
    
    llvm::Function* function = symbol->function;

    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(getContextGenCode()->getBackend()->getContext(), "entry", function);
    getContextGenCode()->getBackend()->getBuilder().SetInsertPoint(entryBlock);

    return function;
}

// auto MethodFunctionDeclarationGenerator::createFunction() -> llvm::Function*
// {
//     std::cout << "MethodFunctionDeclarationGenerator::createFunction\n"; // c'est ici le problème

//     auto& nodeData = getContextGenCode()->getNodeComponentRegistry()->get<NodeDeclarationFunctionComponents>(
//         getNodeDeclarationFunction()->getNodeId()
//     );

//     // peut-etre un problème avec les ID jsp, je penses pas, le nom 'test' s'affiche bien

//     llvm::StringRef functionName = nodeData.getName().value;
//     std::string className = getContextGenCode()->getCurrentClassName();

//     std::cout << "FUNCTION NAME: " << functionName.str() <<"\n";

//     // le problème se situe ici avec un argument je penses

//     auto const& classInfo = getContextGenCode()->getRegistryClass()->get(className);
//     const auto& symbolPtr = classInfo->getRegistryFunctionLocal()->get(functionName);
//     if (!prysma::isa<SymbolFunctionLocal>(symbolPtr.get())) {
//         throw std::runtime_error("Error: Expected SymbolFunctionLocal");
//     }
//     const auto* symbol = prysma::cast<const SymbolFunctionLocal>(symbolPtr.get());
    
//     llvm::Function* function = symbol->function;

//     llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(getContextGenCode()->getBackend()->getContext(), "entry", function);
//     getContextGenCode()->getBackend()->getBuilder().SetInsertPoint(entryBlock);

//     return function;
// }

auto MethodFunctionDeclarationGenerator::createFunction() -> llvm::Function* // OK C'EST PROBABLEMENT UN TRUC DE JE DONNE PAS LE BON CONTEXT GEN CODE
{
  
    auto* ctx = getContextGenCode();

    auto& nodeData = getContextGenCode()->getNodeDataRegistry()->get(getNodeDeclarationFunction());

    llvm::StringRef functionName = nodeData.getName().value;

    std::string className = ctx->getCurrentClassName();

    auto* classRegistry = ctx->getRegistryClass();
 
    auto const& classInfo = classRegistry->get(className);

    auto* functionRegistry = classInfo->getRegistryFunctionLocal();
 
    const auto& symbolPtr = functionRegistry->get(functionName);



    if (!prysma::isa<SymbolFunctionLocal>(symbolPtr.get())) {
        throw std::runtime_error("Error: Expected SymbolFunctionLocal");
    }

    const auto* symbol = prysma::cast<const SymbolFunctionLocal>(symbolPtr.get());

    llvm::Function* function = symbol->function;

    auto& context = ctx->getBackend()->getContext();
    auto& builder = ctx->getBackend()->getBuilder();


    llvm::BasicBlock* entryBlock =
        llvm::BasicBlock::Create(context, "entry", function);

    builder.SetInsertPoint(entryBlock);


    return function;
}

void MethodFunctionDeclarationGenerator::handleConstructedArguments(llvm::Function* function, const ArgumentsCodeGen& args)
{

    std::size_t argIndex = 0;
    
    if (function->arg_size() > 0) {
        llvm::Argument* thisArg = function->getArg(0);
        thisArg->setName("this");
        
        llvm::Type* argType = thisArg->getType();
        llvm::AllocaInst* alloca = getContextGenCode()->getBackend()->getBuilder().CreateAlloca(argType);
        getContextGenCode()->getBackend()->getBuilder().CreateStore(thisArg, alloca);
        
        Token thisToken;
        thisToken.value = "this";
        thisToken.type = TOKEN_IDENTIFIER;
        
        Symbol symboleThis;
        symboleThis = Symbol(alloca, symboleThis.getType(), symboleThis.getPointedElementType());
        symboleThis = Symbol(symboleThis.getAddress(), nullptr, symboleThis.getPointedElementType()); 
        
        getContextGenCode()->getRegistryVariable()->registerVariable(thisToken, symboleThis);
        argIndex = 1;
    }

    for (auto* nodeArg : args.arguments) {
        auto& nodeData = getContextGenCode()->getNodeDataRegistry()->get(nodeArg);

        llvm::Argument* arg = function->getArg(static_cast<unsigned int>(argIndex));
        arg->setName(nodeData.getName().value);
        
        llvm::Type* argType = arg->getType();
        llvm::AllocaInst* alloca = getContextGenCode()->getBackend()->getBuilder().CreateAlloca(argType);
        getContextGenCode()->getBackend()->getBuilder().CreateStore(arg, alloca);
        
        Token argumentToken;
        argumentToken.value = nodeData.getName().value;
        argumentToken.type = TOKEN_IDENTIFIER;
        
        Symbol symbole;
        symbole = Symbol(alloca, symbole.getType(), symbole.getPointedElementType());
        symbole = Symbol(symbole.getAddress(), nodeData.getType(), symbole.getPointedElementType());
        getContextGenCode()->getRegistryVariable()->registerVariable(argumentToken, symbole);
        
        argIndex++;
    }
}

void StandardFunctionDeclarationGenerator::handleConstructedArguments(llvm::Function* function, const ArgumentsCodeGen& args)
{
    std::size_t argIndex = 0;

    for (auto* nodeArg : args.arguments) {
        auto& nodeData = getContextGenCode()->getNodeDataRegistry()->get(nodeArg);

        llvm::Argument* arg = function->getArg(static_cast<unsigned int>(argIndex));
        arg->setName(nodeData.getName().value);
        
        llvm::Type* argType = arg->getType();
        llvm::AllocaInst* alloca = getContextGenCode()->getBackend()->getBuilder().CreateAlloca(argType);
        getContextGenCode()->getBackend()->getBuilder().CreateStore(arg, alloca);
        
        Token argumentToken;
        argumentToken.value = nodeData.getName().value;
        argumentToken.type = TOKEN_IDENTIFIER;
        
        Symbol symbole;
        symbole = Symbol(alloca, symbole.getType(), symbole.getPointedElementType());
        symbole = Symbol(symbole.getAddress(), nodeData.getType(), symbole.getPointedElementType());
        getContextGenCode()->getRegistryVariable()->registerVariable(argumentToken, symbole);
        
        argIndex++;
    }
}

void FunctionDeclarationGenerator::declareFunction()
{
    auto& nodeData = getContextGenCode()->getNodeDataRegistry()->get(getNodeDeclarationFunction());

    llvm::Type* returnType = nodeData.getReturnType()->generateLLVMType(getContextGenCode()->getBackend()->getContext());
    
    ArgumentsCodeGen argumentsCodeGen;
    if (getNodeDeclarationFunction() != nullptr) {
        for (INode* node : nodeData.getArguments()) {
            auto extractor = ArgExtractorFunction{ _contextGenCode };

            node->accept(&extractor);

            if (extractor.getArg() != nullptr) {
                argumentsCodeGen.arguments.push_back(extractor.getArg());

                auto& exctracted_arg_comp = getContextGenCode()->getNodeDataRegistry()->get(extractor.getArg());
                auto *extract_arg_type = exctracted_arg_comp.getType();

                llvm::Type* argType = extract_arg_type->generateLLVMType(getContextGenCode()->getBackend()->getContext()); // ca devrait être extractor

                argumentsCodeGen.argTypes.push_back(argType);
            }
        }
    }

    llvm::Function* function = createFunction();
    
    getContextGenCode()->getReturnContextCompilation()->push(nodeData.getReturnType());
    getContextGenCode()->getRegistryVariable()->push();
    
    handleConstructedArguments(function, argumentsCodeGen);

    nodeData.getBody()->accept(_visitorGeneralCodeGen);

    llvm::BasicBlock* currentBlock = getContextGenCode()->getBackend()->getBuilder().GetInsertBlock();
    if (currentBlock != nullptr && returnType->isVoidTy())
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
        if (currentBlock->getTerminator() == nullptr)
        {
            getContextGenCode()->getBackend()->getBuilder().CreateRetVoid();
        }
#pragma GCC diagnostic pop
    }

    getContextGenCode()->getRegistryVariable()->pop();
    getContextGenCode()->getReturnContextCompilation()->pop();

    getContextGenCode()->setTemporaryValue(Symbol(function, getContextGenCode()->getTemporaryValue().getType(), getContextGenCode()->getTemporaryValue().getPointedElementType()));
}

// Generation of the Call

std::unique_ptr<FunctionCallGenerator> FunctionCallGenerator::create(ContextGenCode* context, IVisitor* visitor)
{
    if (!context->getCurrentClassName().empty()) {
        return std::make_unique<MethodFunctionCallGenerator>(context, visitor);
    }
    return std::make_unique<StandardFunctionCallGenerator>(context, visitor);
}

FunctionCallGenerator::FunctionCallGenerator(ContextGenCode* context, IVisitor* visitor)
    : _contextGenCode(context), _visitorGeneralCodeGen(visitor)
{
}

const SymbolFunctionLocal* MethodFunctionCallGenerator::getLocalFunction(llvm::StringRef functionName)
{
    std::string className = getContextGenCode()->getCurrentClassName();
    auto const& classInfo = getContextGenCode()->getRegistryClass()->get(className);
    if(classInfo->getRegistryFunctionLocal()->exists(functionName)){
        const auto& symbolPtr = classInfo->getRegistryFunctionLocal()->get(functionName);
        if (!prysma::isa<SymbolFunctionLocal>(symbolPtr.get())) {
            throw std::runtime_error("Error: Expected SymbolFunctionLocal");
        }
        return prysma::cast<const SymbolFunctionLocal>(symbolPtr.get());
    }
    
    if (getContextGenCode()->getRegistryFunctionLocal()->exists(functionName)) {
        const auto& symbolPtr = getContextGenCode()->getRegistryFunctionLocal()->get(functionName);
        if (!prysma::isa<SymbolFunctionLocal>(symbolPtr.get())) {
            throw std::runtime_error("Error: Expected SymbolFunctionLocal");
        }
        return prysma::cast<const SymbolFunctionLocal>(symbolPtr.get());
    }

    throw std::runtime_error("Method or function not found in class scope: " + functionName.str());
}

auto StandardFunctionCallGenerator::getLocalFunction(llvm::StringRef functionName) -> const SymbolFunctionLocal*
{

    if (getContextGenCode()->getRegistryFunctionLocal()->exists(functionName)) {
        const auto& symbolPtr = getContextGenCode()->getRegistryFunctionLocal()->get(functionName);
        if (!prysma::isa<SymbolFunctionLocal>(symbolPtr.get())) {
            throw std::runtime_error("Error: Expected SymbolFunctionLocal");
        }
        return prysma::cast<const SymbolFunctionLocal>(symbolPtr.get());
    }

    throw std::runtime_error("Function not found in local registry: " + functionName.str());
}

void FunctionCallGenerator::generateCallFunction(NodeCallFunction* nodeCallFunction)
{

    auto& nodeData = getContextGenCode()->getNodeDataRegistry()->get(nodeCallFunction);

    llvm::StringRef functionName = nodeData.getName().value;

    if (RegistryBuiltIns::isBuiltIn(functionName)) {
        RegistryBuiltIns::generateCall(functionName, nodeCallFunction, getContextGenCode(), _visitorGeneralCodeGen);
        return; 
    }

    getContextGenCode()->getRegistryArgument()->clear();

    const SymbolFunctionLocal* symbolFunction = getLocalFunction(functionName);
    llvm::Function* targetFunction = symbolFunction->function;
    llvm::FunctionType* functionType = targetFunction->getFunctionType();
    
    unsigned int paramIndex = 0; 
    for (INode* argumentChild : nodeData.getChildren()) 
    {
        argumentChild->accept(_visitorGeneralCodeGen);
        llvm::Value* argumentValue = getContextGenCode()->getTemporaryValue().getAddress();

        if (argumentValue == nullptr) {
            throw std::runtime_error("Error: Argument did not generate a value.");
        }

        llvm::Value* finalValue = argumentValue;
        if (paramIndex < functionType->getNumParams()) {
            llvm::Type* expectedType = functionType->getParamType(paramIndex);
            finalValue = getContextGenCode()->getBackend()->createAutoCast(argumentValue, expectedType);
        }

        getContextGenCode()->getRegistryArgument()->add(finalValue);
        paramIndex++;
    }

    std::vector<llvm::Value*> args = getContextGenCode()->getRegistryArgument()->get();
    getContextGenCode()->getRegistryArgument()->clear();

    if(targetFunction->getReturnType()->isVoidTy())
    {
        getContextGenCode()->getBackend()->getBuilder().CreateCall(targetFunction, args);
        getContextGenCode()->setTemporaryValue(Symbol(nullptr, getContextGenCode()->getTemporaryValue().getType(), getContextGenCode()->getTemporaryValue().getPointedElementType()));
        getContextGenCode()->setTemporaryValue(Symbol(getContextGenCode()->getTemporaryValue().getAddress(), nullptr, getContextGenCode()->getTemporaryValue().getPointedElementType()));
        return;
    }
    
    getContextGenCode()->setTemporaryValue(Symbol(getContextGenCode()->getBackend()->getBuilder().CreateCall(
        targetFunction, 
        args, 
        "call_result"
    ), getContextGenCode()->getTemporaryValue().getType(), getContextGenCode()->getTemporaryValue().getPointedElementType()));
    getContextGenCode()->setTemporaryValue(Symbol(getContextGenCode()->getTemporaryValue().getAddress(), symbolFunction->returnType, getContextGenCode()->getTemporaryValue().getPointedElementType()));
}


// Management of Native Functions (Built-ins)

bool RegistryBuiltIns::isBuiltIn(llvm::StringRef name) {

    return name == "print";
}

void RegistryBuiltIns::generateCall(llvm::StringRef name, NodeCallFunction* nodeCallFunction, ContextGenCode* context, IVisitor* visitor) {
    
    auto& nodeData = context->getNodeDataRegistry()->get(nodeCallFunction);
    auto nodeChildren = nodeData.getChildren(); 

    if (name == "print") {
        if (nodeChildren.empty()) {
            return;
        }

        nodeChildren[0]->accept(visitor);
        llvm::Value* argumentValue = context->getTemporaryValue().getAddress();
        IType* argumentType = context->getTemporaryValue().getType();
        
        if (argumentValue == nullptr) {
            throw std::runtime_error("Error: The print argument did not generate a value.");
        }

        char tag = 'i';
        auto& builder = context->getBackend()->getBuilder();

        if (argumentType != nullptr && argumentType->isFloating()) {
            tag = 'f';
            argumentValue = builder.CreateFPExt(argumentValue, builder.getDoubleTy());
        } 
        else if (argumentType != nullptr && argumentType->isBoolean()) {
            tag = 'b';
            argumentValue = builder.CreateZExt(argumentValue, builder.getInt32Ty());
        } 
        else if (argumentType != nullptr && argumentType->isString()) {
            tag = 's';
            if (auto* loadInst = llvm::dyn_cast<llvm::LoadInst>(argumentValue)) {
                argumentValue = loadInst->getPointerOperand();
            }
        }

        llvm::Value* llvmTag = builder.getInt32(static_cast<uint32_t>(tag));
        
        const auto& symbolPtr = context->getRegistryFunctionLocal()->get("print");
        if (!prysma::isa<SymbolFunctionLocal>(symbolPtr.get())) {
            throw std::runtime_error("Error: Expected SymbolFunctionLocal for 'print'");
        }
        const auto* symbolPrint = prysma::cast<const SymbolFunctionLocal>(symbolPtr.get());
        
        builder.CreateCall(symbolPrint->function, { llvmTag, argumentValue });
        context->setTemporaryValue(Symbol(nullptr, context->getTemporaryValue().getType(), context->getTemporaryValue().getPointedElementType()));
    }
}
