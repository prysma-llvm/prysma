//===-- manager_operator.h --------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef FF112DD1_03F3_41D3_8B8F_5E7E64C5467A
#define FF112DD1_03F3_41D3_8B8F_5E7E64C5467A

#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/parser/equation/interfaces/i_manager_operator.h"
#include "compiler/parser/equation/interfaces/i_manager_parenthesis.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include <vector>

/**
 * @class OperatorManager
 * @brief Link in the chain of responsibility for operators
 * Implements the Chain of Responsibility pattern
 */
class OperatorManager : public IOperatorManager {
private:
    IOperatorManager* _next;
    IManagerParenthesis* _parenthesisManager;
    TokenType _tokenType;

public:
    /**
     * @brief Constructor
     * @param tokenType The token type representing this operator
     */
    explicit OperatorManager(TokenType tokenType);
    
    /**
     * @brief Sets the parenthesis manager
     * @param parenthesisManager Parenthesis manager
     */
    void setParenthesisManager(IManagerParenthesis* parenthesisManager);
    
    /**
     * @brief Sets the next link in the chain
     * @param next The next manager
     */
    void setNext(IOperatorManager* next) override;
    
    /**
     * @brief Finds this operator in the equation
     * @param equation The equation to analyze
     * @return The index of the operator, or -1
     */
    PRYSMA_NODISCARD virtual int findOperator(const std::vector<Token>& equation) const;
    
    /**
     * @brief Handles the search for this operator or delegates to the next
     * @param equation The equation to process
     * @return The index of the found operator, or -1
     */
    auto handle(const std::vector<Token>& equation) -> int override;
};

#endif /* FF112DD1_03F3_41D3_8B8F_5E7E64C5467A */
