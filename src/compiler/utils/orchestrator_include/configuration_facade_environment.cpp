//===-- configuration_facade_environment.cpp --------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include "compiler/ast/utils/orchestrator_include/configuration_facade_environment.h"
#include "compiler/ast/registry/data/id_generator.hpp"
#include "compiler/ast/registry/data/node_data_registry.hpp"
#include "compiler/macros/prysma_maybe_unused.h"

#include "compiler/ast/registry/context_gen_code.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/ast/registry/context_parser.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/ast/registry/stack/return_context_compilation.h"
#include "compiler/ast/registry/registry_argument.h"
#include "compiler/ast/registry/registry_class.h"
#include "compiler/ast/registry/registry_expression.h"
#include "compiler/ast/registry/registry_function.h"
#include "compiler/ast/registry/registry_instruction.h"
#include "compiler/ast/registry/registry_type.h"
#include "compiler/ast/registry/types/type_simple.h"
#include "compiler/ast/builder_tree_instruction.h"
#include "compiler/control_memory/parser_delete.h"
#include "compiler/include_module/parser_include.h"
#include "compiler/builder/equation/builder_equation_flottante.h"
#include "compiler/parser/parser_type.h"

// Expressions
#include "compiler/llvm/llvm_backend.h"
#include "compiler/math/expression_literal.h"
#include "compiler/registry/registry_file.h"
#include "compiler/variable/expression_identifiant.h"
#include "compiler/variable/expression_ref_variable.h"
#include "compiler/variable/expression_un_ref_variable.h"
#include "compiler/math/expression_negation.h"
#include "compiler/math/expression_string.h"
#include "compiler/array/expression_array_initialization.h"
#include "compiler/object/expression_call_central.h"
#include "compiler/control_memory/expression_new.h"

// Parsers d'instructions
#include "compiler/function/parser_declaration_function.h"
#include "compiler/variable/parser_assignment_variable.h"
#include "compiler/object/parser_call_central.h"
#include "compiler/variable/parser_declaration_variable.h"
#include "compiler/variable/parser_ref_variable.h"
#include "compiler/variable/parser_un_ref_variable.h"
#include "compiler/function/parser_return.h"
#include "compiler/function/parser_arg_function.h"
#include "compiler/condition/parser_if.h"
#include "compiler/loops/parser_while.h"

#include "compiler/lexer/token_type.h"
#include "compiler/object/parser_class.h"

#include <llvm-18/llvm/IR/DerivedTypes.h>
#include <llvm-18/llvm/IR/Instructions.h>
#include <llvm-18/llvm/IR/Value.h>
#include <llvm/Support/Allocator.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// NOLINTBEGIN(cppcoreguidelines-owning-memory)

ConfigurationFacadeEnvironment::ConfigurationFacadeEnvironment( // all objects coming here are shared between threads 
        //IdGenerator* globalIdGenerator, en attendant de trouver solution // rendre atomic et PROBABLEMENT METTRE SHARDED DEVANT MON REGISTRE, JE PENSES QU'IL DEVRA ETRE GLOBAL COMME LE ID GENERATOR AUSSI
        RegistryFunctionGlobal* registryFunctionGlobale,
        PRYSMA_MAYBE_UNUSED FileRegistry* fileRegistry
    )
    : _registryFunctionGlobal(registryFunctionGlobale),
      // _registryFile(fileRegistry),
      _registryExpression(nullptr),
      _builderTreeInstruction(nullptr),
      _builderEquation(nullptr),
      _parserType(nullptr),
      _contextParser(nullptr),
      _contextExpression(nullptr)
{
}

ConfigurationFacadeEnvironment::~ConfigurationFacadeEnvironment()
{
    if (_contextExpression != nullptr) {
        _contextExpression->~ContextExpression();
    }
    if (_contextParser != nullptr) {
        _contextParser->~ContextParser();
    }
    if (_parserType != nullptr) {
        _parserType->~TypeParser();
    }
    if (_builderEquation != nullptr) {
        _builderEquation->~BuilderFloatEquation();
    }
    if (_builderTreeInstruction != nullptr) {
        _builderTreeInstruction->~BuilderTreeInstruction();
    }
    if (_registryExpression != nullptr) {
        _registryExpression->~RegistryExpression();
    }
}

void ConfigurationFacadeEnvironment::initialize(const std::string& filePath)
{
    createRegistries();
    createGenerators();
    createContext(filePath);
    registerExternalFunctions();
    registerBaseTypes();
    registerExpressions();
    registerInstructions();
}

void ConfigurationFacadeEnvironment::createRegistries()
{
    _backend = std::make_unique<LlvmBackend>();
    _registryInstruction = std::make_unique<RegistryInstruction>();
    _registryVariable = std::make_unique<RegistryVariable>();
    _registryFunctionLocal = std::make_unique<RegistryFunctionLocal>();
    _registryType = std::make_unique<RegistryType>();
    _returnContextCompilation = std::make_unique<ReturnContextCompilation>();
    _registryArgument = std::make_unique<RegistryArgument>();
    _registryClass = std::make_unique<RegistryClass>();
    _nodeDataRegistry = std::make_unique<NodeDataRegistry>();
}

void ConfigurationFacadeEnvironment::createGenerators()
{
    _idGenerator = std::make_unique<IdGenerator>();
}

void ConfigurationFacadeEnvironment::createContext(const std::string& filePath)
{
    Symbol tempValue;
    tempValue = Symbol(nullptr, tempValue.getType(), tempValue.getPointedElementType());
    tempValue = Symbol(tempValue.getAddress(), nullptr, tempValue.getPointedElementType());

    _context = std::make_unique<ContextGenCode>(
        _registryType.get(),
        _backend.get(),
        _nodeDataRegistry.get(),
        
        _idGenerator.get(), // pas totalement certain, je penses qu'il devrait être shared mais le nodeDataRegistry devrait l'être aussi

        _registryInstruction.get(),
        _registryVariable.get(),
        _registryFunctionGlobal,
        _registryFunctionLocal.get(),
        _returnContextCompilation.get(),
        _registryArgument.get(),
        _registryClass.get(),
        tempValue,
        &_arena,
        filePath
    );
}

void ConfigurationFacadeEnvironment::registerExternalFunctions()
{
    // Create a real Void type for Prysma
    IType* prysmaVoidType = new (_arena.Allocate<TypeSimple>()) TypeSimple(llvm::Type::getVoidTy(_context->getBackend()->getContext())); // NOLINT(cppcoreguidelines-owning-memory)

    // backSlashN
    _context->getBackend()->declareExternal("backSlashN", llvm::Type::getVoidTy(_context->getBackend()->getContext()), {});
    {
        auto symBackSlashNGlobal = std::make_unique<SymbolFunctionGlobal>();
        symBackSlashNGlobal->returnType = prysmaVoidType;
        symBackSlashNGlobal->functionName = "backSlashN";
        symBackSlashNGlobal->isBuiltin = true;
        _context->getRegistryFunctionGlobal()->registerElement("backSlashN", std::move(symBackSlashNGlobal));

        auto symBackSlashNLocal = std::make_unique<SymbolFunctionLocal>();
        symBackSlashNLocal->function = _context->getBackend()->getModule().getFunction("backSlashN");
        symBackSlashNLocal->returnType = prysmaVoidType;
        symBackSlashNLocal->node = nullptr;
        _context->getRegistryFunctionLocal()->registerElement("backSlashN", std::move(symBackSlashNLocal));
    }

    // print
    std::vector<llvm::Type*> print_args;
    print_args.push_back(llvm::Type::getInt32Ty(_context->getBackend()->getContext()));
    llvm::FunctionType* print_type = llvm::FunctionType::get(llvm::Type::getVoidTy(_context->getBackend()->getContext()), print_args, true);
    llvm::Function* printFunc = llvm::Function::Create(print_type, llvm::Function::ExternalLinkage, "print", _context->getBackend()->getModule());
    {
        auto symPrintGlobal = std::make_unique<SymbolFunctionGlobal>();
        symPrintGlobal->returnType = prysmaVoidType;
        symPrintGlobal->functionName = "print";
        symPrintGlobal->isBuiltin = true;
        // print also takes an i32, but we mock it
        _context->getRegistryFunctionGlobal()->registerElement("print", std::move(symPrintGlobal));

        auto symPrintLocal = std::make_unique<SymbolFunctionLocal>();
        symPrintLocal->function = printFunc;
        symPrintLocal->returnType = prysmaVoidType;
        symPrintLocal->node = nullptr;
        _context->getRegistryFunctionLocal()->registerElement("print", std::move(symPrintLocal));
    }

    // prysma_malloc
    std::vector<llvm::Type*> malloc_args;
    malloc_args.push_back(llvm::Type::getInt64Ty(_context->getBackend()->getContext()));
    llvm::FunctionType* malloc_type = llvm::FunctionType::get(llvm::PointerType::getUnqual(_context->getBackend()->getContext()), malloc_args, false);
    llvm::Function* mallocFunc = llvm::Function::Create(malloc_type, llvm::Function::ExternalLinkage, "prysma_malloc", _context->getBackend()->getModule());
    {
        auto symMallocGlobal = std::make_unique<SymbolFunctionGlobal>();
        symMallocGlobal->returnType = static_cast<TypeSimple*>(static_cast<void*>(new (_arena.Allocate<TypeSimple>()) TypeSimple(llvm::PointerType::getUnqual(_context->getBackend()->getContext()))));
        symMallocGlobal->functionName = "prysma_malloc";
        symMallocGlobal->isBuiltin = true;
        _context->getRegistryFunctionGlobal()->registerElement("prysma_malloc", std::move(symMallocGlobal));

        auto symMallocLocal = std::make_unique<SymbolFunctionLocal>();
        symMallocLocal->function = mallocFunc;
        symMallocLocal->returnType = new (_arena.Allocate<TypeSimple>()) TypeSimple(llvm::PointerType::getUnqual(_context->getBackend()->getContext())); // NOLINT(cppcoreguidelines-owning-memory)
        symMallocLocal->node = nullptr;
        _context->getRegistryFunctionLocal()->registerElement("prysma_malloc", std::move(symMallocLocal));
    }

    // prysma_free
    std::vector<llvm::Type*> free_args;
    free_args.push_back(llvm::PointerType::getUnqual(_context->getBackend()->getContext()));
    llvm::FunctionType* free_type = llvm::FunctionType::get(llvm::Type::getVoidTy(_context->getBackend()->getContext()), free_args, false);
    llvm::Function* freeFunc = llvm::Function::Create(free_type, llvm::Function::ExternalLinkage, "prysma_free", _context->getBackend()->getModule());
    {
        auto symFreeGlobal = std::make_unique<SymbolFunctionGlobal>();
        symFreeGlobal->returnType = prysmaVoidType;
        symFreeGlobal->functionName = "prysma_free";
        symFreeGlobal->isBuiltin = true;
        _context->getRegistryFunctionGlobal()->registerElement("prysma_free", std::move(symFreeGlobal));

        auto symFreeLocal = std::make_unique<SymbolFunctionLocal>();
        symFreeLocal->function = freeFunc;
        symFreeLocal->returnType = prysmaVoidType;
        symFreeLocal->node = nullptr;
        _context->getRegistryFunctionLocal()->registerElement("prysma_free", std::move(symFreeLocal));
    }
}

void ConfigurationFacadeEnvironment::registerBaseTypes()
{
    _context->getRegistryType()->registerElement(TOKEN_TYPE_STRING, llvm::Type::getInt8Ty(_context->getBackend()->getContext()));
    _context->getRegistryType()->registerElement(TOKEN_TYPE_CHAR, llvm::Type::getInt8Ty(_context->getBackend()->getContext()));
    _context->getRegistryType()->registerElement(TOKEN_TYPE_INT64, llvm::Type::getInt64Ty(_context->getBackend()->getContext()));
    _context->getRegistryType()->registerElement(TOKEN_TYPE_INT32, llvm::Type::getInt32Ty(_context->getBackend()->getContext()));
    _context->getRegistryType()->registerElement(TOKEN_TYPE_FLOAT, llvm::Type::getFloatTy(_context->getBackend()->getContext()));
    _context->getRegistryType()->registerElement(TOKEN_TYPE_BOOL, llvm::Type::getInt1Ty(_context->getBackend()->getContext()));
    _context->getRegistryType()->registerElement(TOKEN_TYPE_VOID, llvm::Type::getVoidTy(_context->getBackend()->getContext()));
    _context->getRegistryType()->registerElement(TOKEN_TYPE_PTR, llvm::PointerType::getUnqual(_context->getBackend()->getContext()));
}

void ConfigurationFacadeEnvironment::createContextParser()
{
    if (_builderTreeInstruction == nullptr || _builderEquation == nullptr || _parserType == nullptr) {
        throw std::logic_error("ContextParser cannot be created before tree builders and type parser");
    }

    ContextParser::Dependencies deps = {
        _builderEquation->getBuilderTree(),
        _builderTreeInstruction,
        _parserType,
        _registryVariable.get(),
        _registryType.get(),
        _nodeDataRegistry.get(),
        _idGenerator.get()
    };
    _contextParser = new (_arena.Allocate<ContextParser>()) ContextParser(deps); // NOLINT(cppcoreguidelines-owning-memory)
}

void ConfigurationFacadeEnvironment::registerExpressions()
{
    // Build the orchestrators of the abstract syntax tree
    _builderTreeInstruction = new (_arena) // NOLINT(cppcoreguidelines-owning-memory)
        BuilderTreeInstruction(_registryInstruction.get(), _nodeDataRegistry.get(), _idGenerator.get(), _arena); 

    _registryExpression = new (_arena.Allocate<RegistryExpression>()) RegistryExpression(); // NOLINT(cppcoreguidelines-owning-memory)

    _builderEquation = new (_arena) // NOLINT(cppcoreguidelines-owning-memory)
        BuilderFloatEquation(_registryExpression, _nodeDataRegistry.get(), _idGenerator.get(), _arena);

    // Create the TypeParser with the registry
    _parserType = new (_arena.Allocate<TypeParser>()) // NOLINT(cppcoreguidelines-owning-memory)
        TypeParser(_context->getRegistryType(), _nodeDataRegistry.get(), _builderEquation->getBuilderTree());

    if (_contextParser == nullptr) {
        createContextParser();
    }

    _contextExpression = new (_arena.Allocate<ContextExpression>()) ContextExpression( // NOLINT(cppcoreguidelines-owning-memory)
        _builderEquation->getBuilderTree(),
        _builderTreeInstruction,
        _parserType,
        _contextParser,
        &_arena,
        _registryVariable.get(),
        _registryType.get(),
        _nodeDataRegistry.get(),
        
        _context->getIdGenerator() // pas certain totalement, voir la note plus haut
    );

    auto* exprLitInt = new (_arena.Allocate<ExpressionLiteral>()) ExpressionLiteral(*_contextExpression); // // NOLINT(cppcoreguidelines-owning-memory)
    _registryExpression->registerElement(TOKEN_LIT_INT, exprLitInt);

    auto* exprLitFloat = new (_arena.Allocate<ExpressionLiteral>()) ExpressionLiteral(*_contextExpression); // NOLINT(cppcoreguidelines-owning-memory)
    _registryExpression->registerElement(TOKEN_LIT_FLOAT, exprLitFloat);

    auto* exprLitBool = new (_arena.Allocate<ExpressionLiteral>()) ExpressionLiteral(*_contextExpression); // NOLINT(cppcoreguidelines-owning-memory)
    _registryExpression->registerElement(TOKEN_LIT_BOOL, exprLitBool);

    auto* exprIdentifier = new (_arena.Allocate<ExpressionIdentifiant>()) ExpressionIdentifiant(*_contextExpression); // NOLINT(cppcoreguidelines-owning-memory)
    _registryExpression->registerElement(TOKEN_IDENTIFIER, exprIdentifier);

    auto* exprRef = new (_arena.Allocate<ExpressionRefVariable>()) ExpressionRefVariable(*_contextExpression); // NOLINT(cppcoreguidelines-owning-memory)
    _registryExpression->registerElement(TOKEN_REF, exprRef);

    auto* exprUnRef = new (_arena.Allocate<ExpressionUnRefVariable>()) ExpressionUnRefVariable(*_contextExpression); // NOLINT(cppcoreguidelines-owning-memory)
    _registryExpression->registerElement(TOKEN_UNREF, exprUnRef);

    auto* exprNeg = new (_arena.Allocate<ExpressionNegation>()) ExpressionNegation(*_contextExpression); // NOLINT(cppcoreguidelines-owning-memory)
    _registryExpression->registerElement(TOKEN_NOT, exprNeg);

    auto* exprString = new (_arena.Allocate<ExpressionString>()) ExpressionString(*_contextExpression); // NOLINT(cppcoreguidelines-owning-memory)
    _registryExpression->registerElement(TOKEN_QUOTE, exprString);

    auto* exprArray = new (_arena.Allocate<ExpressionArrayInitialization>()) ExpressionArrayInitialization(*_contextExpression); // NOLINT(cppcoreguidelines-owning-memory)
    _registryExpression->registerElement(TOKEN_BRACKET_OPEN, exprArray);

    auto* exprCall = new (_arena.Allocate<ExpressionCallCentral>()) ExpressionCallCentral(*_contextExpression); // NOLINT(cppcoreguidelines-owning-memory)
    _registryExpression->registerElement(TOKEN_CALL, exprCall);

    auto* exprNew = new (_arena.Allocate<ExpressionNew>()) ExpressionNew(*_contextExpression); // NOLINT(cppcoreguidelines-owning-memory)
    _registryExpression->registerElement(TOKEN_NEW, exprNew);
}

void ConfigurationFacadeEnvironment::registerInstructions()
{
    auto* parsFonc = new (_arena.Allocate<ParserDeclarationFunction>()) ParserDeclarationFunction(*_contextParser); // NOLINT(cppcoreguidelines-owning-memory)
    _registryInstruction->registerElement(TOKEN_FUNCTION, parsFonc);

    auto* parsAff = new (_arena.Allocate<ParserAssignmentVariable>()) ParserAssignmentVariable(*_contextParser); // NOLINT(cppcoreguidelines-owning-memory)
    _registryInstruction->registerElement(TOKEN_ASSIGN, parsAff);

    auto* parsDec = new (_arena.Allocate<ParserDeclarationVariable>()) ParserDeclarationVariable(*_contextParser); // NOLINT(cppcoreguidelines-owning-memory)
    _registryInstruction->registerElement(TOKEN_DECL, parsDec);

    auto* parsCall = new (_arena.Allocate<ParserCallCentral>()) ParserCallCentral(*_contextParser); // NOLINT(cppcoreguidelines-owning-memory)
    _registryInstruction->registerElement(TOKEN_CALL, parsCall);

    auto* parsRet = new (_arena.Allocate<ParserReturn>()) ParserReturn(*_contextParser); // NOLINT(cppcoreguidelines-owning-memory)
    _registryInstruction->registerElement(TOKEN_RETURN, parsRet);

    auto* parsArg = new (_arena.Allocate<ParserArgFunction>()) ParserArgFunction(*_contextParser); // NOLINT(cppcoreguidelines-owning-memory)
    _registryInstruction->registerElement(TOKEN_ARG, parsArg);

    auto* parsUnRef = new (_arena.Allocate<ParserUnRefVariable>()) ParserUnRefVariable(*_contextParser); // NOLINT(cppcoreguidelines-owning-memory)
    _registryInstruction->registerElement(TOKEN_UNREF, parsUnRef);

    auto* parsRefVar = new (_arena.Allocate<ParserRefVariable>()) ParserRefVariable(*_contextParser); // NOLINT(cppcoreguidelines-owning-memory)
    _registryInstruction->registerElement(TOKEN_REF, parsRefVar);

    auto* parsIf = new (_arena.Allocate<ParserIf>()) ParserIf(*_contextParser); // NOLINT(cppcoreguidelines-owning-memory)
    _registryInstruction->registerElement(TOKEN_IF, parsIf);

    auto* parsWhile = new (_arena.Allocate<ParserWhile>()) ParserWhile(*_contextParser); // NOLINT(cppcoreguidelines-owning-memory)
    _registryInstruction->registerElement(TOKEN_WHILE, parsWhile);

    auto* parsInclude = new (_arena.Allocate<ParserInclude>()) ParserInclude(*_contextParser); // NOLINT(cppcoreguidelines-owning-memory)
    _registryInstruction->registerElement(TOKEN_INCLUDE, parsInclude);

    auto* parsDelete = new (_arena.Allocate<ParserDelete>()) ParserDelete(*_contextParser); // NOLINT(cppcoreguidelines-owning-memory)
    _registryInstruction->registerElement(TOKEN_DELETE, parsDelete);

    auto* parsClass = new (_arena.Allocate<ParserClass>()) ParserClass(*_contextParser); // NOLINT(cppcoreguidelines-owning-memory)
    _registryInstruction->registerElement(TOKEN_CLASS, parsClass);
}

auto ConfigurationFacadeEnvironment::getContext() const -> ContextGenCode*
{
    return _context.get();
}

auto ConfigurationFacadeEnvironment::getArena() -> llvm::BumpPtrAllocator&
{
    return _arena;
}

auto ConfigurationFacadeEnvironment::getBuilderTreeInstruction() const -> BuilderTreeInstruction*
{
    return _builderTreeInstruction;
}

auto ConfigurationFacadeEnvironment::getBuilderEquation() const -> BuilderFloatEquation*
{
    return _builderEquation;
}
// NOLINTEND(cppcoreguidelines-owning-memory)
