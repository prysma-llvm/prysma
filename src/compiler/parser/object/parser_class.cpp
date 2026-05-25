//===-- parser_class.cpp ----------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef PARSER_CLASS_CPP
#define PARSER_CLASS_CPP

#include "../build/generationCode/include/compiler/ast/node_data.hpp"
#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/manager_error.h"
#include "compiler/ast/ast_genere.h"
#include "compiler/object/parser_class.h"
#include "compiler/ast/nodes/interfaces/i_node.h"
#include "compiler/ast/registry/context_parser.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include "compiler/visitor/interfaces/i_visitor.h"
#include "compiler/utils/prysma_cast.h"
#include <iostream>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/FormatVariadic.h>
#include <vector>
#include <cstddef>

namespace
{
  constexpr int DEFAULT_CLASS_VECTOR_SIZE = 8;
  struct TokenClassName { Token t; };
  struct TokenVisibility { Token t; };

  class ClassParameters {
  public:
    ClassParameters(const TokenClassName& classNameToken, const TokenVisibility& currentVisibility)
        : classNameToken_(classNameToken.t), current_visibility_(currentVisibility.t) {}

    PRYSMA_NODISCARD auto classNameToken() const -> const Token& { return classNameToken_; }
    PRYSMA_NODISCARD auto current_visibility() const -> const Token& { return current_visibility_; }

  private:
    Token classNameToken_;
    Token current_visibility_;
  };

  void classifyClassNode(INode* node,
              const ClassParameters& param, 
              ContextParser& contextParser,
              llvm::SmallVectorImpl<INode*>& memberList,
              llvm::SmallVectorImpl<INode*>& builders)
  {
      if (node == nullptr) {
        return;
      }

      // ce système fonctionne bien mais il repose sur un système de "pseudo_casting" qui coute cher au runtime.
      // il serait plus judicieux d'utiliser les informations du type courant depuis le wrapper crtp. 
      // Le truc principal est que ce dyn_cast introduit un static_cast qui pourait être évité avec le crtp.

      // Il faut absolument refactoriser ce bout de code du moment que le crtp est en place. Ça fera du bien.

      if (auto* declarationVariable = prysma::dyn_cast<NodeDeclarationVariable>(node)) {
        if (declarationVariable != nullptr) {
          auto& nodeData = contextParser.getNodeDataRegistry()->get(declarationVariable);

          node = contextParser.getBuilderTreeInstruction()->allocate<NodeDeclarationVariable>(contextParser.getIdGenerator()->next());

          contextParser.getNodeDataRegistry()->construct_for<DeclarationVariableNodeData>( // car il est INode*, il s'agirait de dyn_cast mais je vous laisse le faire
              node, 
              param.current_visibility(),
              nodeData.getName(),
              nodeData.getType(),
              nodeData.getExpression()
          );
        }
        memberList.push_back(node);
        return;
      }

      if (auto* declarationFunction = prysma::dyn_cast<NodeDeclarationFunction>(node)) {
        if (declarationFunction != nullptr) {
          auto& nodeData = contextParser.getNodeDataRegistry()->get(declarationFunction);

          node = contextParser.getBuilderTreeInstruction()->allocate<NodeDeclarationFunction>(contextParser.getIdGenerator()->next());

          contextParser.getNodeDataRegistry()->construct_for<DeclarationFunctionNodeData>(
              node,
              param.current_visibility(),
              nodeData.getReturnType(),
              nodeData.getName(),
              nodeData.getArguments(),
              nodeData.getBody()
          );

          std::cout << "parser_class.cpp\n"; // ICI AUSSI
        }
        
        auto* newDeclarationFunction = prysma::cast<NodeDeclarationFunction>(node); // très suspect, à changer avec le crtp
        auto& nodeData = contextParser.getNodeDataRegistry()->get(newDeclarationFunction);
        // il se pourrait que le node data soit empty et que le registre lance un exception (not found). il faudrait peut-être emplace ou revoir l'algo pour être certain...

        if (newDeclarationFunction != nullptr && nodeData.getName().value == param.classNameToken().value) {
          builders.push_back(node);
          return;
        }
        
        memberList.push_back(node);
        return;
      }

    throw CompilationError(llvm::formatv("Invalid class member: '{0}'", param.classNameToken().value).str(), Line(param.classNameToken().line), Column(param.classNameToken().column));
  }
}


ParserClass::ParserClass(ContextParser& contextParser) 
    : _contextParser(contextParser)
{}

ParserClass::~ParserClass()
= default;

// Example: class ClassName : parent
//          { 
//              private: 
//                  dec int64 privateAttribute;
//                  dec string privateAttribute2;
//                  fn void method(arg int64 param, arg string param2)
//                  {
//                      aff privateAttribute = param;
//                      aff privateAttribute2 = param2;
//                  }
//              public:
//                  fn void ClassName(arg int64 param, arg string param2)
//                  {
//                      aff privateAttribute = param;
//                      aff privateAttribute2 = param2;
//                  }
//                  fn void method(arg int64 param, arg string[] param2)
//                  {
//                      aff privateAttribute = param;
//                      aff privateAttribute2 = param2;
//                  }
//           }

auto ParserClass::parse(std::vector<Token>& tokens, std::size_t& index) -> INode*
{
    consume(tokens, index, TOKEN_CLASS, "Expected 'class' at the beginning of the class declaration.");
    Token classNameToken = consume(tokens, index, TOKEN_IDENTIFIER, "Expected an identifier after 'class' for the class name.");
    
    // Inheritance management: class ClassName : Parent
    llvm::SmallVector<INode*, DEFAULT_CLASS_VECTOR_SIZE> inheritance; // TODO: handle multiple inheritance 
    
    
    consume(tokens, index, TOKEN_BRACE_OPEN, "Expected '{' after the class name.");
    llvm::SmallVector<INode*, DEFAULT_CLASS_VECTOR_SIZE> memberList;
    llvm::SmallVector<INode*, DEFAULT_CLASS_VECTOR_SIZE> builders;
    
    // By default, start in PRIVATE section if no visibility keyword
    Token current_visibility;
    current_visibility.type = TOKEN_PRIVATE;
    current_visibility.value = "private";

    while (index < static_cast<int>(tokens.size()) && tokens[index].type != TOKEN_BRACE_CLOSE) {
        TokenType tokenType = tokens[index].type;

        if (tokenType == TOKEN_PUBLIC) {
            current_visibility = tokens[index];
            consume(tokens, index, TOKEN_PUBLIC, "Expected 'public' for the public section of the class.");
            consume(tokens, index, TOKEN_COLON, "Expected ':' after 'public'.");
            continue;
        }

        if (tokenType == TOKEN_PRIVATE) {
            current_visibility = tokens[index];
            consume(tokens, index, TOKEN_PRIVATE, "Expected 'private' for the private section of the class.");
            consume(tokens, index, TOKEN_COLON, "Expected ':' after 'private'.");
            continue;
        }

        if (tokenType == TOKEN_PROTECTED) {
          current_visibility = tokens[index];
          consume(tokens, index, TOKEN_PROTECTED, "Expected 'protected' for the protected section of the class.");
          consume(tokens, index, TOKEN_COLON, "Expected ':' after 'protected'.");
          continue;
        }

        // If no visibility keyword, parse members in the current section (private by default)
        INode* node = _contextParser.getBuilderTreeInstruction()->build(tokens, index);
        TokenClassName tokenClassName;
        tokenClassName.t = classNameToken;
        TokenVisibility tokenVisibility{current_visibility};
        ClassParameters classParameters(tokenClassName, tokenVisibility);

        classifyClassNode(node, classParameters, _contextParser, memberList, builders);
    }

    consume(tokens, index, TOKEN_BRACE_CLOSE, "Expected '}' at the end of the class declaration.");

    auto* nodeClass = _contextParser.getBuilderTreeInstruction()->allocate<NodeClass>(_contextParser.getIdGenerator()->next()); 

    _contextParser.getNodeDataRegistry()->construct(
        nodeClass,
        _contextParser.getBuilderTreeInstruction()->allocateArray<INode*>(inheritance),
        _contextParser.getBuilderTreeInstruction()->allocateArray<INode*>(memberList), 
        _contextParser.getBuilderTreeInstruction()->allocateArray<INode*>(builders), 
        classNameToken
    );

    return nodeClass;
}

#endif /* PARSER_CLASS_CPP */




