//===-- expression_data_registry.hpp ----------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#pragma once

#include "compiler/ast/registry/data/multi_storage_registry.hpp"

//using ExpressionDataRegistry = MultiStorageRegistry<ExpressionRegistryTable, DefaultHandleProvider, 1 << 12>;

// TODO: Quand il y aura assez d'expressions dans le système, il sera pertinent de faire un registre réservé à cet effet.
//       pour l'instant, il est plus intelligent de laisser les données du seul noeud Expression (Operation) dans le registre
//       de nodeData. Si nécessaire, le système est implémenté de façon à permettre la création de registres personnalisés. D'ailleurs,
//       j'ai laissé un petit exemple en haut de ce que la définition pourrait ressembler. En cas de questionnement, je vous réfère au
//       fichier node_data_registry.hpp qui contient une définition complète et fonctionnelle. Vous pourrez vous en inspirer.
