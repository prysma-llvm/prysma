//===-- service_parenthesis.h -----------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef DA3AD58B_D7B7_44D3_AC56_E1DDCD78F54F
#define DA3AD58B_D7B7_44D3_AC56_E1DDCD78F54F

#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/parser/equation/interfaces/i_manager_parenthesis.h"
#include "compiler/ast/registry/registry_symbole.h"
#include "compiler/lexer/lexer.h"
#include <vector>

class RegistrySymbol;

class ParenthesisService : public IManagerParenthesis {
private:
    RegistrySymbol* _registrySymbol;  
    /**
     * @brief Checks if a pair of parentheses is wrapping
     * @param equation The equation to check
     * @return true if the parentheses wrap the entire expression
     */
    static auto isWrapping(const std::vector<Token>& equation) -> bool;
    
    /**
     * @brief Checks if an operator at a given position is unary
     * @param equation The equation
     * @param index The index of the operator
     * @return true if it is a unary sign
     */
    PRYSMA_NODISCARD auto isUnarySign(const std::vector<Token>& equation, int index) const -> bool;

public:
    /**
     * @brief Constructor
     * @param symbolRegistry Operator registry
     */
    explicit ParenthesisService(RegistrySymbol* symbolRegistry);
    
    /**
     * @brief Removes parentheses that wrap the entire expression
     * @param equation The equation to process
     * @return The equation without wrapping parentheses
     */
    auto removeWrappingParentheses(const std::vector<Token>& equation) -> std::vector<Token> override;
    
    /**
     * @brief Finds the last operator at parenthesis level zero
     * @param equation The equation to analyze
     * @param op The operator token to search for
     * @return The index of the operator, or -1 if not found
     */
    auto findLastAtLevelZero(const std::vector<Token>& equation, Token op) -> int override;
};

#endif /* DA3AD58B_D7B7_44D3_AC56_E1DDCD78F54F */
