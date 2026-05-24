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

TEST_CASE("AST Registry - Sequential Construction", "[ast][registry][pmu]") {
    constexpr size_t NUM_NODES = 20000;
    
    std::vector<std::unique_ptr<NodeLiteral>> nodes;
    nodes.reserve(NUM_NODES);
    for (size_t i = 0; i < NUM_NODES; ++i) {
        nodes.push_back(std::make_unique<NodeLiteral>(i));
    }

    // Mesure de la création séquentielle des données dans le registre
    prctl(PR_TASK_PERF_EVENTS_ENABLE, 0, 0, 0, 0);

    for (int iter = 0; iter < 100; ++iter) {
        NodeDataRegistry registry;
        for (size_t i = 0; i < NUM_NODES; ++i) {
            Token t{TOKEN_LIT_INT, "42", 1, 1};
            registry.construct(nodes[i].get(), t);
        }
    }

    prctl(PR_TASK_PERF_EVENTS_DISABLE, 0, 0, 0, 0);
}

TEST_CASE("AST Registry - Sequential Access", "[ast][registry][pmu]") {
    constexpr size_t NUM_NODES = 20000;
    
    std::vector<std::unique_ptr<NodeLiteral>> nodes;
    nodes.reserve(NUM_NODES);
    NodeDataRegistry registry;
    for (size_t i = 0; i < NUM_NODES; ++i) {
        nodes.push_back(std::make_unique<NodeLiteral>(i));
        Token t{TOKEN_LIT_INT, "42", 1, 1};
        registry.construct(nodes.back().get(), t);
    }

    // Idéal pour le DOD: Accès parfaitement séquentiel
    prctl(PR_TASK_PERF_EVENTS_ENABLE, 0, 0, 0, 0);

    for (int iter = 0; iter < 1000; ++iter) {
        size_t sum = 0;
        for (size_t i = 0; i < NUM_NODES; ++i) {
            auto& data = registry.get(nodes[i].get());
            sum += data.getToken().line; 
        }
        (void)sum; 
    }

    prctl(PR_TASK_PERF_EVENTS_DISABLE, 0, 0, 0, 0);
}

TEST_CASE("AST Registry - Random Access (Visitor Pattern Simulation)", "[ast][registry][pmu]") {
    constexpr size_t NUM_NODES = 20000;
    
    std::vector<std::unique_ptr<NodeLiteral>> nodes;
    nodes.reserve(NUM_NODES);
    std::vector<NodeLiteral*> shuffled_ptrs;
    shuffled_ptrs.reserve(NUM_NODES);
    
    NodeDataRegistry registry;
    for (size_t i = 0; i < NUM_NODES; ++i) {
        nodes.push_back(std::make_unique<NodeLiteral>(i));
        Token t{TOKEN_LIT_INT, "42", 1, 1};
        registry.construct(nodes.back().get(), t);
        shuffled_ptrs.push_back(nodes.back().get());
    }

    // Mélange pour simuler les sauts mémoires (cache misses) de l'arbre lors d'un parcours Visitor
    std::mt19937 rng(42);
    std::shuffle(shuffled_ptrs.begin(), shuffled_ptrs.end(), rng);

    // Ce test met en évidence la perte de performance due aux indirections mémoire
    prctl(PR_TASK_PERF_EVENTS_ENABLE, 0, 0, 0, 0);

    for (int iter = 0; iter < 1000; ++iter) {
        size_t sum = 0;
        for (size_t i = 0; i < NUM_NODES; ++i) {
            auto& data = registry.get(shuffled_ptrs[i]);
            sum += data.getToken().line;
        }
        (void)sum;
    }

    prctl(PR_TASK_PERF_EVENTS_DISABLE, 0, 0, 0, 0);
}
