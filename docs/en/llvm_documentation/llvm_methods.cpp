//===-- llvmmethod_doc.cpp --------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

// LLVM includes
#include "compiler/ast/register/context_gen_code.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>

#include <llvm/IR/Type.h>
#include <llvm/IR/Constants.h>  
#include <llvm/IR/Instructions.h>
#include "compiler/llvm/llvm_serializer.h"

#include <llvm/TargetParser/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

using namespace llvm;

// Compilation command for the example, used to better understand the LLVM framework
//g++ -fdiagnostics-color=always -g -O0 -std=c++17 -I../include LLVMMethodeDoc.cpp ../src/Compiler/LLVM/LLVMSerializer.cpp $(llvm-config --cxxflags --ldflags --libs) -o LLVMMethodeDoc

// ===== LLVM Initialization =====
llvm::LLVMContext context;
llvm::Module module("output", context);
llvm::IRBuilder<> builder(context);

AllocaInst* exampleVariableInitialization()
{
    // Type for the int variable
    Type * typeInt = builder.getInt32Ty();
    Value * arraySize = nullptr; 
    
    // Create memory space for variable x (int32)
    AllocaInst *allocaX = builder.CreateAlloca(typeInt, arraySize, "x");

    // Initialize the value in x (store), i.e., int x = 10
    int value = 10;
    Value *valConst = ConstantInt::get(typeInt, value);
    builder.CreateStore(valConst, allocaX);
    return allocaX; 
}

// ===== Example usage: read, add 5, and write back =====
void readValueAndAdd5(AllocaInst* allocaX)
{
    Type * typeInt = builder.getInt32Ty();
    const Twine NameTmp = "loadtmp"; 
    
    // Use instructions like CreateAdd to create an intermediate assembly instruction to perform
    // addition, multiplication, division, subtraction.
    Value *loaded = builder.CreateLoad(typeInt, allocaX, NameTmp);
    Value *added = builder.CreateAdd(loaded, ConstantInt::get(typeInt, 5), NameTmp);
    builder.CreateStore(added, allocaX);
}

void saveCode()
{
    LLVMSerializer fileProcessor{context, module};
    fileProcessor.SaveLLVMCode("output.ll");
}

void functionDeclaration()
{
    // 1. Signature: void (i32, float)
    std::vector<llvm::Type*> args = { builder.getInt32Ty(), builder.getFloatTy() };
    llvm::FunctionType* ft = llvm::FunctionType::get(builder.getVoidTy(), args, false);

    // 2. Creation
    llvm::Function* function = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "myFunction", module);

    // 3. Create the block
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(context, "entry", function);
    builder.SetInsertPoint(entryBlock);

    // 4. Copy arguments (Boilerplate)
    auto argIt = function->arg_begin();
    llvm::Value* valX = argIt++; // Get the 1st arg
    llvm::Value* valY = argIt++; // Get the 2nd arg

    // For X
    llvm::Value* ptrX = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "ptr_x");
    builder.CreateStore(valX, ptrX);

    // For Y
    llvm::Value* ptrY = builder.CreateAlloca(builder.getFloatTy(), nullptr, "ptr_y");
    builder.CreateStore(valY, ptrY);

    // 5. End
    builder.CreateRetVoid();
}

int main()
{
    // Target configuration, avoids having random memory addresses, which causes random crashes.
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmPrinters();

    std::string targetTriple = sys::getDefaultTargetTriple();
    module.setTargetTriple(targetTriple);

    std::string error;
    auto target = TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        errs() << error;
        return 1;
    }

    TargetOptions opt;
    auto targetMachine = target->createTargetMachine(targetTriple, "generic", "", opt, Reloc::Model::PIC_);
    
    // Set the correct memory layout (standard alignment and address space 0)
    module.setDataLayout(targetMachine->createDataLayout());
    // ------------------------------------------------------------------

    // Everything must be inside a function before execution.
    // 1. Define the function type (void func())
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(context), false);
    
    // 2. Create the "main" function in the module
    Function *mainFunc = Function::Create(ftype, Function::ExternalLinkage, "main", module);
    
    // 3. Create a BasicBlock named "entry" inside this function
    BasicBlock *bb = BasicBlock::Create(context, "entry", mainFunc);
    
    // 4. Set the builder's cursor in this block (CRUCIAL)
    builder.SetInsertPoint(bb);

    // Now the builder knows where to write
    AllocaInst* allocX = exampleVariableInitialization();
    readValueAndAdd5(allocX);
    
    // Always end a block with a return (void here) for it to be valid
    builder.CreateRetVoid();

    saveCode();

    return 0; 
}
