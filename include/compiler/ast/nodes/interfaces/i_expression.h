//===-- i_expression.h ------------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include "i_node.h"

/**
 * @interface IExpression
 * @brief Interface for nodes of an expression tree
 * Inherits from INode
 */
class IExpression : public INode {
public:
    IExpression() = default;
    ~IExpression() override = default;

    IExpression(const IExpression&) = delete;
    auto operator=(const IExpression&) -> IExpression& = delete;
    IExpression(IExpression&&) = delete;
    auto operator=(IExpression&&) -> IExpression& = delete;
    
    /**
     * @brief Adds two expressions as left and right children
     * @param left Left child expression
     * @param right Right child expression
     * @return Reference to the current node
     */
    // virtual auto addExpression(
    //     INode* left, 
    //     INode* right
    // ) -> IExpression* = 0;

    // NOTE: Dans la nouvelle architecture, les noeuds et les expressions ne sont que des identifiants. 
    //       Ainsi, les classes INode et IExpression ne sont plus nécessaires outre le contract de getNodeTypeGenerated
    //       car les données se trouvent en réalité dans les registres prévus à cet effet. Ainsi, cette classe ne contient
    //       plus addExpression. Il n'est pas nécessaire et est déja présent dans les data du noeud. D'ailleurs, j'ai laissé
    //       une petite note dans expression_data_registry.hpp afin que vous puissiez comprendre l'état du nouveau système.
    //       Pour l'instant, la fonction addExpression est dans les données. Il est vrai que addExpression est un contract et
    //       qu'il fait office de setter, ce qui est un peu inhabituel pour une classe de donnée (data) mais c'est en attendant 
    //       que la situation devienne plus stable. Je vous réfère au fichier précédemment mentionné afin que vous compreniez
    //       mieux l'enjeux de la situation.
};
