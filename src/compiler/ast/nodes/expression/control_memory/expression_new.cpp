//===-- expression_new.cpp --------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef EXPRESSION_NEW_CPP
#define EXPRESSION_NEW_CPP

#include "compiler/control_memory/expression_new.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/manager_error.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include <llvm/ADT/SmallVector.h>
#include <cstddef>
#include <vector>

ExpressionNew::ExpressionNew(ContextExpression& expressionContext)
    : _context(expressionContext)
{}

ExpressionNew::~ExpressionNew()
= default;

auto ExpressionNew::build(std::vector<Token>& equation) -> INode*
{
    std::size_t index = 0;
    if (equation.empty() || equation[0].type != TOKEN_NEW) {
        throw CompilationError("Error: the 'new' token is expected", Line(0), Column(0));
    }
    index++;

    if (index >= equation.size()) {
        throw CompilationError("Error: no valid type for the object created with 'new'", Line(equation[0].line), Column(equation[0].column));
    }

    Token typeName = equation[index];

    if (typeName.type != TOKEN_TYPE_INT32 &&
        typeName.type != TOKEN_TYPE_INT64 &&
        typeName.type != TOKEN_TYPE_FLOAT &&
        typeName.type != TOKEN_TYPE_BOOL &&
        typeName.type != TOKEN_TYPE_CHAR &&
        typeName.type != TOKEN_TYPE_STRING &&
        typeName.type != TOKEN_TYPE_VOID &&
        typeName.type != TOKEN_TYPE_PTR &&
        typeName.type != TOKEN_IDENTIFIER) {
        throw CompilationError("Error: no valid type for the object created with 'new'", Line(typeName.line), Column(typeName.column));
    }

    // We need to fill the arguments of new (e.g., new MyClass(arg1, arg2)) by adding the childs of nodeNew
    llvm::SmallVector<INode*, 4> arguments;
    if (typeName.type == TOKEN_IDENTIFIER) {
        index++; // Skip the type name
        
        if (index < equation.size() && equation[index].type == TOKEN_PAREN_OPEN) {
            index++; // Skip '('

            while(index < equation.size() && equation[index].type != TOKEN_PAREN_CLOSE) {
                arguments.push_back(_context.getContextParser()->getBuilderTreeEquation()->build(equation, index));

                if (index < equation.size() && equation[index].type == TOKEN_COMMA) {
                    index++; // Skip the comma
                }
            }
            if (index < equation.size() && equation[index].type == TOKEN_PAREN_CLOSE) {
                index++; // Skip ')'
            }
        }
    }

    auto* nodeNew = _context.getBuilderTreeEquation()->allocate<NodeNew>(_context.getIdGenerator()->next());

    _context.getNodeDataRegistry()->construct(
        nodeNew,
        _context.getBuilderTreeEquation()->allocateArray<INode*>(arguments),
        typeName
    );
    
    return nodeNew;
}

#endif /* EXPRESSION_NEW_CPP */
