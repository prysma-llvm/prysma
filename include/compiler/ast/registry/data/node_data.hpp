//===-- node_data.hpp -------------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/types/i_type.h"
#include "compiler/lexer/lexer.h"
#include <llvm-18/llvm/ADT/ArrayRef.h>

struct InstructionNodeData {
private:
    llvm::ArrayRef<INode*> children;

public:
    InstructionNodeData() = default; 

    explicit InstructionNodeData(llvm::ArrayRef<INode*> p_children = nullptr)
        : children(p_children) {}

    llvm::ArrayRef<INode*> getChildren() { return children; }
};

struct FunctionCallNodeData {
private:
    Token nomFunction;
    llvm::ArrayRef<INode*> children;

public:
    FunctionCallNodeData(Token p_nomFunction, llvm::ArrayRef<INode*> p_children = nullptr)
        : nomFunction(p_nomFunction), children(p_children) {}

    Token getName() { return nomFunction; }
    llvm::ArrayRef<INode*> getChildren() { return children; }
};

struct FunctionArgNodeData {
private:
    IType* type;
    Token nom;

public:
    FunctionArgNodeData(IType* p_type, Token p_nom)
        : type(p_type), nom(p_nom) {}

    IType* getType() { return type; }
    Token getName() { return nom; }
};

struct FunctionDeclarationNodeData {
private:
    Token visibilite;
    IType* typeReturn;
    Token nom;
    llvm::ArrayRef<INode*> arguments;
    INode* body;

public:
    FunctionDeclarationNodeData(Token p_visibilite, IType* p_typeReturn, Token p_nom,
                                llvm::ArrayRef<INode*> p_arguments = nullptr, INode* p_body = nullptr)
        : visibilite(p_visibilite),
          typeReturn(p_typeReturn),
          nom(p_nom),
          arguments(p_arguments),
          body(p_body) {}

    Token getVisibility() { return visibilite; }
    IType* getReturnType() { return typeReturn; }
    Token getName() { return nom; }
    llvm::ArrayRef<INode*> getArguments() { return arguments; }
    INode* getBody() { return body; }
};

struct ReturnNodeData {
private:
    INode* valeurReturn;

public:
    ReturnNodeData(INode* p_valeurReturn = nullptr)
        : valeurReturn(p_valeurReturn) {}

    INode* getReturnValue() { return valeurReturn; }
};

struct VariableAssignmentNodeData {
private:
    Token nom;
    INode* expression;
    Token token;

public:
    VariableAssignmentNodeData(Token p_nom, INode* p_expression, Token p_token)
        : nom(p_nom), expression(p_expression), token(p_token) {}

    Token getName() { return nom; }
    INode* getExpression() { return expression; }
    Token getToken() { return token; }
};

struct VariableDeclarationNodeData {
private:
    Token visibilite;
    Token nom;
    IType* type;
    INode* expression;

public:
    VariableDeclarationNodeData(Token p_visibilite, Token p_nom, IType* p_type = nullptr, INode* p_expression = nullptr)
        : visibilite(p_visibilite), nom(p_nom), type(p_type), expression(p_expression) {}

    Token getVisibility() { return visibilite; }
    Token getName() { return nom; }
    IType* getType() { return type; }
    INode* getExpression() { return expression; }
};

struct VariableRefNodeData {
private:
    Token nomVariable;

public:
    VariableRefNodeData(Token p_nomVariable)
        : nomVariable(p_nomVariable) {}

    Token getName() { return nomVariable; }
};

struct VariableUnrefNodeData {
private:
    Token nomVariable;

public:
    VariableUnrefNodeData(Token p_nomVariable)
        : nomVariable(p_nomVariable) {}

    Token getName() { return nomVariable; }
};

struct IdentifierNodeData {
public:
    IdentifierNodeData() = default;

};

struct ArrayAssignmentNodeData {
private:
    Token nom;
    INode* expressionIndex;
    INode* expression;
    Token token;

public:
    ArrayAssignmentNodeData(Token p_nom, INode* p_expressionIndex,
                            INode* p_expression, Token p_token)
        : nom(p_nom),
          expressionIndex(p_expressionIndex),
          expression(p_expression),
          token(p_token) {}

    Token getName() { return nom; }
    INode* getExpressionIndex() { return expressionIndex; }
    INode* getExpression() { return expression; }
    Token getToken() { return token; }
};

struct ArrayInitializationNodeData {
private:
    llvm::ArrayRef<INode*> elements;

public:
    ArrayInitializationNodeData(llvm::ArrayRef<INode*> p_elements = nullptr)
        : elements(p_elements) {}

    llvm::ArrayRef<INode*> getElements() { return elements; }
};

struct ClassNodeData {
private:
    llvm::ArrayRef<INode*> heritage;
    llvm::ArrayRef<INode*> listMembers;
    llvm::ArrayRef<INode*> builder;
    Token nomClass;

public:
    ClassNodeData(llvm::ArrayRef<INode*> p_heritage,
                  llvm::ArrayRef<INode*> p_listMembers,
                  llvm::ArrayRef<INode*> p_builder,
                  Token p_nomClass)
        : heritage(p_heritage),
          listMembers(p_listMembers),
          builder(p_builder),
          nomClass(p_nomClass) {}

    llvm::ArrayRef<INode*> getInheritance() { return heritage; }
    llvm::ArrayRef<INode*> getMembers() { return listMembers; }
    llvm::ArrayRef<INode*> getBuilder() { return builder; }
    Token getName() { return nomClass; }
};

struct ArrayReadingNodeData {
private:
    INode* indexEquation;
    Token nomArray;

public:
    ArrayReadingNodeData(INode* p_indexEquation, Token p_nomArray)
        : indexEquation(p_indexEquation), nomArray(p_nomArray) {}

    INode* getIndexEquation() { return indexEquation; }
    Token getName() { return nomArray; }
};

struct ObjectCallNodeData {
private:
    Token nomObject;
    Token nomMethode;
    llvm::ArrayRef<INode*> children;

public:
    ObjectCallNodeData(Token p_nomObject, Token p_nomMethode,
                       llvm::ArrayRef<INode*> p_children = nullptr)
        : nomObject(p_nomObject),
          nomMethode(p_nomMethode),
          children(p_children) {}

    Token getObjectName() { return nomObject; }
    Token getMethodName() { return nomMethode; }
    llvm::ArrayRef<INode*> getChildren() { return children; }
};

struct AccessAttributeNodeData {
private:
    Token nomObject;
    Token nomAttribute;

public:
    AccessAttributeNodeData(Token p_nomObject, Token p_nomAttribute)
        : nomObject(p_nomObject), nomAttribute(p_nomAttribute) {}

    Token getObjectName() { return nomObject; }
    Token getAttributeName() { return nomAttribute; }
};

struct ObjectDeclarationNodeData {
private:
    Token nomObject;
    IType* typeObject;

public:
    ObjectDeclarationNodeData(Token p_nomObject, IType* p_typeObject = nullptr)
        : nomObject(p_nomObject), typeObject(p_typeObject) {}

    Token getObjectName() { return nomObject; }
    IType* getObjectType() { return typeObject; }
};

struct IfNodeData {
private:
    INode* nodeCondition;
    INode* nodeBlocIf;
    INode* nodeBlocElse;
    INode* nodeBlocEndif;

public:
    IfNodeData(INode* p_nodeCondition = nullptr, INode* p_nodeBlocIf = nullptr,
               INode* p_nodeBlocElse = nullptr, INode* p_nodeBlocEndif = nullptr)
        : nodeCondition(p_nodeCondition),
          nodeBlocIf(p_nodeBlocIf),
          nodeBlocElse(p_nodeBlocElse),
          nodeBlocEndif(p_nodeBlocEndif) {}

    INode* getNodeCondition() { return nodeCondition; }
    INode* getNodeBlocIf() { return nodeBlocIf; }
    INode* getNodeBlocElse() { return nodeBlocElse; }
    INode* getNodeBlocEndif() { return nodeBlocEndif; }
};

struct NewNodeData {
private:
    llvm::ArrayRef<INode*> arguments;
    Token nomType;

public:
    NewNodeData(llvm::ArrayRef<INode*> p_arguments, Token p_nomType)
        : arguments(p_arguments), nomType(p_nomType) {}

    llvm::ArrayRef<INode*> getArguments() { return arguments; }
    Token getName() { return nomType; }
};

struct DeleteNodeData {
private:
    Token nomType;

public:
    DeleteNodeData(Token p_nomType)
        : nomType(p_nomType) {}

    Token getName() { return nomType; }
};

struct IncludeNodeData {
private:
    Token path;

public:
    IncludeNodeData(Token p_path)
        : path(p_path) {}

    Token getPath() { return path; }
};

struct WhileNodeData {
private:
    INode* nodeCondition;
    INode* nodeBlocWhile;
    INode* nodeBlocFinWhile;

public:
    WhileNodeData(INode* p_nodeCondition = nullptr, INode* p_nodeBlocWhile = nullptr,
                  INode* p_nodeBlocFinWhile = nullptr)
        : nodeCondition(p_nodeCondition),
          nodeBlocWhile(p_nodeBlocWhile),
          nodeBlocFinWhile(p_nodeBlocFinWhile) {}

    INode* getNodeCondition() { return nodeCondition; }
    INode* getNodeWhileBlock() { return nodeBlocWhile; }
    INode* getNodeWhileEndBlock() { return nodeBlocFinWhile; }
};

struct OperationNodeData { // il est expression je pense
private:
    Token token;
    INode* gauche;
    INode* droite;

public:
    OperationNodeData(Token p_token, INode* p_gauche = nullptr, INode* p_droite = nullptr)
        : token(p_token), gauche(p_gauche), droite(p_droite) {}
        
    Token getToken() { return token; }
    INode* getLeft() { return gauche; }
    INode* getRight() { return droite; }

public:
    void addExpression(INode* left, INode* right) { gauche = left; droite = right; } // spécifique à expression

};

struct LiteralNodeData {
private:
    Token token;

public:
    LiteralNodeData(Token p_token)
        : token(p_token) {}

    Token getToken() { return token; }
};

struct NegationNodeData {
private:
    Token operateur;
    INode* operande;

public:
    NegationNodeData(Token p_operateur, INode* p_operande = nullptr)
        : operateur(p_operateur), operande(p_operande) {}

    Token getOperator() { return operateur; }
    INode* getOperand() { return operande; }
};

struct StringNodeData {
public:
    StringNodeData() = default;

};