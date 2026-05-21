//===-- chain_of_responsibility.h -------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef A58A7095_E8A1_4CAC_A3A8_3136BE3D2B27
#define A58A7095_E8A1_4CAC_A3A8_3136BE3D2B27

#include "compiler/macros/prysma_nodiscard.h"
#include "compiler/parser/equation/interfaces/i_manager_operator.h"
#include "compiler/parser/equation/interfaces/i_manager_parenthesis.h"
#include "compiler/lexer/lexer.h"
#include <cstddef>
#include <llvm/ADT/SmallVector.h>
#include <vector>

class OperatorManager;

constexpr size_t MAX_OPERATORS = 16;

/**
 * @class ChainOfResponsibility
 * @brief Manages the chain of responsibility for operator detection
 * Determines operator precedence
 */
class ChainOfResponsibility {
private:
    IOperatorManager* _start;
    IManagerParenthesis* _parenthesisManager;
    llvm::SmallVector<OperatorManager*, MAX_OPERATORS> _operators;

public:
    /**
     * @brief Constructor
     * @param parenthesisManager Parenthesis management manager
     * @param operators Vector of operator managers in priority order
     */
    ChainOfResponsibility(
        IManagerParenthesis* parenthesisManager, 
        llvm::SmallVector<OperatorManager*, MAX_OPERATORS> operators
    );
    
    /**
     * @brief Destructor
     */
    ~ChainOfResponsibility();

    ChainOfResponsibility(const ChainOfResponsibility&) = delete;
    auto operator=(const ChainOfResponsibility&) -> ChainOfResponsibility& = delete;
    ChainOfResponsibility(ChainOfResponsibility&&) = delete;
    auto operator=(ChainOfResponsibility&&) -> ChainOfResponsibility& = delete;
    
    /**
     * @brief Finds the next operator to process according to priority
     * @param equation The equation to analyze
     * @return The index of the operator, or -1 if none found
     */
    PRYSMA_NODISCARD auto findOperator(const std::vector<Token>& equation) const -> int;
};

#endif /* A58A7095_E8A1_4CAC_A3A8_3136BE3D2B27 */
