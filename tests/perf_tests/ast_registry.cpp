#include "catch.hpp"
#include "compiler/ast/registry/data/node_data_registry.hpp"
#include "compiler/ast/ast_genere.h"
#include "compiler/lexer/token_type.h"
#include "compiler/lexer/lexer.h"
#include <linux/prctl.h>
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
#include <memory>
#include <sys/prctl.h>
#include <variant>

TEST_CASE("AST Registry - Sequential Construction", "[ast][registry][pmu]") {
    constexpr size_t NUM_NODES_PER_TYPE = 250000;
    
    std::vector<std::unique_ptr<NodeLiteral>> literals;
    std::vector<std::unique_ptr<NodeReturn>> returns;
    std::vector<std::unique_ptr<NodeWhile>> whiles;
    std::vector<std::unique_ptr<NodeIf>> ifs;
    
    for (size_t i = 0; i < NUM_NODES_PER_TYPE; ++i) {
        literals.push_back(std::make_unique<NodeLiteral>(i));
        returns.push_back(std::make_unique<NodeReturn>(i + NUM_NODES_PER_TYPE));
        whiles.push_back(std::make_unique<NodeWhile>(i + 2 * NUM_NODES_PER_TYPE));
        ifs.push_back(std::make_unique<NodeIf>(i + 3 * NUM_NODES_PER_TYPE));
    }

    // Mesure de la création séquentielle des données dans le registre
    prctl(PR_TASK_PERF_EVENTS_ENABLE, 0, 0, 0, 0);

    for (int iter = 0; iter < 100; ++iter) {
        NodeDataRegistry registry;
        
        // Construction DOD Pure : on remplit tableau par tableau (zéro saut de contexte)
        for (size_t i = 0; i < NUM_NODES_PER_TYPE; ++i) {
            Token t{TOKEN_LIT_INT, "42", 1, 1};
            registry.construct(literals[i].get(), t);
        }
        for (size_t i = 0; i < NUM_NODES_PER_TYPE; ++i) {
            registry.construct(returns[i].get(), nullptr);
        }
        for (size_t i = 0; i < NUM_NODES_PER_TYPE; ++i) {
            registry.construct(whiles[i].get(), nullptr, nullptr, nullptr);
        }
        for (size_t i = 0; i < NUM_NODES_PER_TYPE; ++i) {
            registry.construct(ifs[i].get(), nullptr, nullptr, nullptr, nullptr);
        }
    }

    prctl(PR_TASK_PERF_EVENTS_DISABLE, 0, 0, 0, 0);
}

TEST_CASE("AST Registry - Sequential Access", "[ast][registry][pmu]") {
    constexpr size_t NUM_NODES_PER_TYPE = 250000;
    
    std::vector<std::unique_ptr<NodeLiteral>> literals;
    std::vector<std::unique_ptr<NodeReturn>> returns;
    std::vector<std::unique_ptr<NodeWhile>> whiles;
    std::vector<std::unique_ptr<NodeIf>> ifs;
    
    NodeDataRegistry registry;
    for (size_t i = 0; i < NUM_NODES_PER_TYPE; ++i) {
        literals.push_back(std::make_unique<NodeLiteral>(i));
        Token t{TOKEN_LIT_INT, "42", 1, 1};
        registry.construct(literals.back().get(), t);
        
        returns.push_back(std::make_unique<NodeReturn>(i + NUM_NODES_PER_TYPE));
        registry.construct(returns.back().get(), nullptr);
        
        whiles.push_back(std::make_unique<NodeWhile>(i + 2 * NUM_NODES_PER_TYPE));
        registry.construct(whiles.back().get(), nullptr, nullptr, nullptr);
        
        ifs.push_back(std::make_unique<NodeIf>(i + 3 * NUM_NODES_PER_TYPE));
        registry.construct(ifs.back().get(), nullptr, nullptr, nullptr, nullptr);
    }

    // Idéal pour le DOD: Accès parfaitement séquentiel par bloc
    prctl(PR_TASK_PERF_EVENTS_ENABLE, 0, 0, 0, 0);

    for (int iter = 0; iter < 1000; ++iter) {
        size_t sum = 0;
        
        // On lit tout le tableau A, puis tout le tableau B, etc.
        for (size_t i = 0; i < NUM_NODES_PER_TYPE; ++i) {
            auto& data = registry.get(literals[i].get());
            sum += reinterpret_cast<size_t>(&data) & 0xFF;
        }
        for (size_t i = 0; i < NUM_NODES_PER_TYPE; ++i) {
            auto& data = registry.get(returns[i].get());
            sum += reinterpret_cast<size_t>(&data) & 0xFF;
        }
        for (size_t i = 0; i < NUM_NODES_PER_TYPE; ++i) {
            auto& data = registry.get(whiles[i].get());
            sum += reinterpret_cast<size_t>(&data) & 0xFF;
        }
        for (size_t i = 0; i < NUM_NODES_PER_TYPE; ++i) {
            auto& data = registry.get(ifs[i].get());
            sum += reinterpret_cast<size_t>(&data) & 0xFF;
        }
        (void)sum; 
    }

    prctl(PR_TASK_PERF_EVENTS_DISABLE, 0, 0, 0, 0);
}

TEST_CASE("AST Registry - Random Access (Visitor Pattern Simulation)", "[ast][registry][pmu]") {
    constexpr size_t NUM_NODES = 1000000;
    
    using NodeVar = std::variant<NodeLiteral*, NodeReturn*, NodeWhile*, NodeIf*>;
    std::vector<std::unique_ptr<INode>> raw_nodes;
    raw_nodes.reserve(NUM_NODES);
    std::vector<NodeVar> shuffled_ptrs;
    shuffled_ptrs.reserve(NUM_NODES);
    
    NodeDataRegistry registry;
    for (size_t i = 0; i < NUM_NODES; ++i) {
        if (i % 4 == 0) {
            auto node = std::make_unique<NodeLiteral>(i);
            Token t{TOKEN_LIT_INT, "42", 1, 1};
            registry.construct(node.get(), t);
            shuffled_ptrs.push_back(node.get());
            raw_nodes.push_back(std::move(node));
        } else if (i % 4 == 1) {
            auto node = std::make_unique<NodeReturn>(i);
            registry.construct(node.get(), nullptr);
            shuffled_ptrs.push_back(node.get());
            raw_nodes.push_back(std::move(node));
        } else if (i % 4 == 2) {
            auto node = std::make_unique<NodeWhile>(i);
            registry.construct(node.get(), nullptr, nullptr, nullptr);
            shuffled_ptrs.push_back(node.get());
            raw_nodes.push_back(std::move(node));
        } else {
            auto node = std::make_unique<NodeIf>(i);
            registry.construct(node.get(), nullptr, nullptr, nullptr, nullptr);
            shuffled_ptrs.push_back(node.get());
            raw_nodes.push_back(std::move(node));
        }
    }

    // Mélange pour simuler les sauts mémoires (cache misses) de l'arbre lors d'un parcours Visitor
    // Le visiteur va sauter entre 4 tableaux (SmartStorage) différents, forçant le CPU à jongler entre 4 zones mémoires
    std::mt19937 rng(42);
    std::shuffle(shuffled_ptrs.begin(), shuffled_ptrs.end(), rng);

    // Ce test met en évidence la perte de performance due aux indirections mémoire
    prctl(PR_TASK_PERF_EVENTS_ENABLE, 0, 0, 0, 0);

    for (int iter = 0; iter < 1000; ++iter) {
        size_t sum = 0;
        for (size_t i = 0; i < NUM_NODES; ++i) {
            // std::visit simule le "dynamic_cast" ou le double-dispatch du Visitor
            std::visit([&](auto* ptr) {
                // Le compilateur résout automatiquement le bon aiguilleur grâce au type de `ptr`
                auto& data = registry.get(ptr);
                // Petite opération pour éviter que le compilateur supprime la boucle
                sum += reinterpret_cast<size_t>(&data) & 0xFF;
            }, shuffled_ptrs[i]);
        }
        (void)sum;
    }

    prctl(PR_TASK_PERF_EVENTS_DISABLE, 0, 0, 0, 0);
}
