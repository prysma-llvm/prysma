//===-- test_ast.cpp --------------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#include <memory>
#include <string>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>
#include <llvm-18/llvm/Support/Allocator.h>
#include <llvm/Support/TargetSelect.h>
#include "compiler/ast/registry/data/node_data_registry.hpp"
#include "compiler/ast/registry/data/id_generator.hpp"
#include "compiler/ast/registry/registry_type.h"
#include "compiler/lexer/token_type.h"
#include "catch.hpp"
#include "compiler/lexer/lexer.h"
#include "compiler/ast/builder_tree_instruction.h"
#include "compiler/builder/equation/builder_equation_flottante.h"
#include "compiler/ast/registry/registry_instruction.h"
#include "compiler/ast/registry/registry_expression.h"
#include "compiler/ast/registry/context_expression.h"
#include "compiler/ast/registry/context_parser.h"
#include "compiler/ast/registry/stack/registry_variable.h"
#include "compiler/ast/nodes/interfaces/i_node.h"

// Expressions
#include "compiler/math/expression_literal.h"
#include "compiler/parser/parser_type.h"
#include "compiler/variable/expression_identifiant.h"
#include "compiler/variable/expression_ref_variable.h"
#include "compiler/variable/expression_un_ref_variable.h"
#include "compiler/math/expression_negation.h"
#include "compiler/math/expression_string.h"
#include "compiler/array/expression_array_initialization.h"
#include "compiler/function/expression_call_function.h"

// Parsers d'instructions
#include "compiler/function/parser_declaration_function.h"
#include "compiler/variable/parser_assignment_variable.h"
#include "compiler/function/parser_call_function.h"
#include "compiler/variable/parser_declaration_variable.h"
#include "compiler/variable/parser_ref_variable.h"
#include "compiler/variable/parser_un_ref_variable.h"
#include "compiler/function/parser_return.h"
#include "compiler/function/parser_arg_function.h"
#include "compiler/condition/parser_if.h"
#include "compiler/loops/parser_while.h"
#include "compiler/include_module/parser_include.h"

#include <list>

using namespace std;

// NOTE: L'architecture est à refaire afin de passer correctement l'accès au registre de données de noeuds. 
//       J'ai patché le système mais ce n'est pas propre. Il fonctionne mais il sera à adapter.

struct EnvironnementAST {
    std::list<std::string> codes;
    llvm::BumpPtrAllocator arena;
    std::unique_ptr<RegistryInstruction> registryInstruction;
    std::unique_ptr<RegistryExpression> registryExpression;
    std::unique_ptr<RegistryType> registryType;
    std::unique_ptr<RegistryVariable> registryVariable;

    std::unique_ptr<NodeDataRegistry> nodeDataRegistry;
    std::unique_ptr<IdGenerator> idGenerator;

    BuilderTreeInstruction* builderTree = nullptr;
    BuilderFloatEquation* builderEquation = nullptr;
    TypeParser* parserType = nullptr;
    ContextParser* contextParser = nullptr;
    ContextExpression* contexteExpression = nullptr;

    EnvironnementAST() {
    
        registryInstruction = std::make_unique<RegistryInstruction>();
        registryExpression = std::make_unique<RegistryExpression>();
        registryType = std::make_unique<RegistryType>();
        registryVariable = std::make_unique<RegistryVariable>();

        nodeDataRegistry = std::make_unique<NodeDataRegistry>();
        idGenerator = std::make_unique<IdGenerator>();

        // Builder d'tree d'instruction 
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmismatched-new-delete"
        builderTree = new (arena) BuilderTreeInstruction(registryInstruction.get(), nodeDataRegistry.get(), idGenerator.get(), arena);
        #pragma GCC diagnostic pop

        //  Strategie d'équation 
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmismatched-new-delete"
        builderEquation = new (arena) BuilderFloatEquation(registryExpression.get(), nodeDataRegistry.get(), idGenerator.get(), arena);
        #pragma GCC diagnostic pop

        parserType = new (arena.Allocate<TypeParser>()) TypeParser(registryType.get(), nodeDataRegistry.get(), builderEquation->getBuilderTree());

        // Créer le ContextParser
        contextParser = new (arena.Allocate<ContextParser>()) ContextParser(ContextParser::Dependencies{
            builderEquation->getBuilderTree(),
            builderTree,
            parserType,
            registryVariable.get(),
            registryType.get(),
            nodeDataRegistry.get(),
            idGenerator.get()
        });

        contexteExpression = new (arena.Allocate<ContextExpression>()) ContextExpression(
            builderEquation->getBuilderTree(),
            builderTree,
            parserType,
            contextParser,
            &arena,
            registryVariable.get(),
            registryType.get(),
            nodeDataRegistry.get(),
            idGenerator.get()
        );

        auto* exprLitInt = new (arena.Allocate<ExpressionLiteral>()) ExpressionLiteral(*contexteExpression);
        registryExpression->registerElement(TOKEN_LIT_INT, exprLitInt);

        auto* exprLitFloat = new (arena.Allocate<ExpressionLiteral>()) ExpressionLiteral(*contexteExpression);
        registryExpression->registerElement(TOKEN_LIT_FLOAT, exprLitFloat);

        auto* exprLitBool = new (arena.Allocate<ExpressionLiteral>()) ExpressionLiteral(*contexteExpression);
        registryExpression->registerElement(TOKEN_LIT_BOOL, exprLitBool);

        auto* exprIdentifiant = new (arena.Allocate<ExpressionIdentifiant>()) ExpressionIdentifiant(*contexteExpression);
        registryExpression->registerElement(TOKEN_IDENTIFIER, exprIdentifiant);

        auto* exprRef = new (arena.Allocate<ExpressionRefVariable>()) ExpressionRefVariable(*contexteExpression);
        registryExpression->registerElement(TOKEN_REF, exprRef);

        auto* exprUnRef = new (arena.Allocate<ExpressionUnRefVariable>()) ExpressionUnRefVariable(*contexteExpression);
        registryExpression->registerElement(TOKEN_UNREF, exprUnRef);

        auto* exprNeg = new (arena.Allocate<ExpressionNegation>()) ExpressionNegation(*contexteExpression);
        registryExpression->registerElement(TOKEN_NOT, exprNeg);

        auto* exprString = new (arena.Allocate<ExpressionString>()) ExpressionString(*contexteExpression);
        registryExpression->registerElement(TOKEN_QUOTE, exprString);

        auto* exprTab = new (arena.Allocate<ExpressionArrayInitialization>()) ExpressionArrayInitialization(*contexteExpression);
        registryExpression->registerElement(TOKEN_BRACKET_OPEN, exprTab);

        auto* exprCall = new (arena.Allocate<ExpressionCallFunction>()) ExpressionCallFunction(*contexteExpression);
        registryExpression->registerElement(TOKEN_CALL, exprCall);

        // Parsers d'instructions
        auto* parsFonc = new (arena.Allocate<ParserDeclarationFunction>()) ParserDeclarationFunction(*contextParser);
        registryInstruction->registerElement(TOKEN_FUNCTION, parsFonc);

        auto* parsAff = new (arena.Allocate<ParserAssignmentVariable>()) ParserAssignmentVariable(*contextParser);
        registryInstruction->registerElement(TOKEN_ASSIGN, parsAff);

        auto* parsDec = new (arena.Allocate<ParserDeclarationVariable>()) ParserDeclarationVariable(*contextParser);
        registryInstruction->registerElement(TOKEN_DECL, parsDec);

        auto* parsCall = new (arena.Allocate<ParserCallFunction>()) ParserCallFunction(*contextParser);
        registryInstruction->registerElement(TOKEN_CALL, parsCall);

        auto* parsRet = new (arena.Allocate<ParserReturn>()) ParserReturn(*contextParser);
        registryInstruction->registerElement(TOKEN_RETURN, parsRet);

        auto* parsArg = new (arena.Allocate<ParserArgFunction>()) ParserArgFunction(*contextParser);
        registryInstruction->registerElement(TOKEN_ARG, parsArg);

        auto* parsUnRef = new (arena.Allocate<ParserUnRefVariable>()) ParserUnRefVariable(*contextParser);
        registryInstruction->registerElement(TOKEN_UNREF, parsUnRef);

        auto* parsRefVar = new (arena.Allocate<ParserRefVariable>()) ParserRefVariable(*contextParser);
        registryInstruction->registerElement(TOKEN_REF, parsRefVar);

        auto* parsIf = new (arena.Allocate<ParserIf>()) ParserIf(*contextParser);
        registryInstruction->registerElement(TOKEN_IF, parsIf);

        auto* parsWhile = new (arena.Allocate<ParserWhile>()) ParserWhile(*contextParser);
        registryInstruction->registerElement(TOKEN_WHILE, parsWhile);

        auto* parsInclude = new (arena.Allocate<ParserInclude>()) ParserInclude(*contextParser);
        registryInstruction->registerElement(TOKEN_INCLUDE, parsInclude);
    }
};


/// Construit un tree d'équation à partir d'une string de code source.
INode* construireEquationDepuisString(EnvironnementAST& env, const std::string& code) {
    env.codes.push_back(code);
    Lexer lexer;
    std::vector<Token> tokens = lexer.tokenize(env.codes.back());

    // Retirer le token EOF ajouté par le Lexer
    if (!tokens.empty() && tokens.back().type == TOKEN_EOF) {
        tokens.pop_back();
    }

    return env.builderEquation->build(tokens);
}

/// Construit un tree d'instructions à partir d'une string de code source.
INode* construireTreeDepuisString(EnvironnementAST& env, const std::string& code) {
    env.codes.push_back(code);
    Lexer lexer;
    std::vector<Token> tokens = lexer.tokenize(env.codes.back());
    return env.builderTree->build(tokens);
}

template<typename TypeAttendu, typename TypeInputr>

TypeAttendu* verifierTypeEtCaster(TypeInputr* node) {
    auto* nodeCast = dynamic_cast<TypeAttendu*>(node);
    REQUIRE(nodeCast != nullptr);
    return nodeCast;
}


template<auto getter, typename TypeAttendu, typename TypeValeur, auto accessor = nullptr> // done
struct matcherNode
{
    TypeValeur valeurAttendue;
    EnvironnementAST& env;
    
    matcherNode(EnvironnementAST& env, TypeValeur p_valeurAttendue) : env(env), valeurAttendue(std::move(p_valeurAttendue)) {}

    template<typename typeBase>
    void operator()(typeBase* node) {
       auto* nodeCast = verifierTypeEtCaster<TypeAttendu>(node);
        auto& nodeData = env.nodeDataRegistry->get(nodeCast);

       if constexpr (accessor == nullptr) {
           REQUIRE((nodeData.*getter)() == valeurAttendue);
       } else {
           REQUIRE(((nodeData.*getter)().*accessor) == valeurAttendue);
       }
    }
};

// Utilisation de la technique fonctor
template<typename TypeCible, auto getter, typename TypeValeur, auto accessor, typename Gauche, typename Droite>
struct matcherGeneralBinaire
{
    Gauche verificateurGauche;
    Droite verificateurDroite;
    TypeValeur valeurAttendue;

    EnvironnementAST& env;

    matcherGeneralBinaire(EnvironnementAST& env, const TypeValeur& p_valeur, Gauche p_nodeGaucheAttendu, Droite p_nodeDroiteAttendu) 
        : env(env), verificateurGauche(std::move(p_nodeGaucheAttendu)), verificateurDroite(std::move(p_nodeDroiteAttendu)), valeurAttendue(p_valeur) {}

    template<typename typeBase>
    void operator()(typeBase* node) {
        auto* nodeOperation = verifierTypeEtCaster<TypeCible>(node);
        auto& nodeData = env.nodeDataRegistry->get(nodeOperation);
        
        if constexpr (accessor == nullptr) {
            REQUIRE((nodeData.*getter)() == valeurAttendue);
        } else {
            REQUIRE(((nodeData.*getter)().*accessor) == valeurAttendue);
        }
        verificateurDroite(nodeData.getRight());
        verificateurGauche(nodeData.getLeft());
    }
};

// Matcher pour une liste d'child d'un node instruction ou autre
template<typename TypeCible, auto getter, typename... Matchers>
struct matcherListsChild 
{
    std::tuple<Matchers...> verificateurs;

    EnvironnementAST& env;

    matcherListsChild(EnvironnementAST& env, Matchers... p_verificateurs) : env(env), verificateurs(std::make_tuple(std::move(p_verificateurs)...)) {}

    template<typename typeBase>
    void operator()(typeBase* node) {
        auto* nodeCast = verifierTypeEtCaster<TypeCible>(node);
        auto& nodeData = env.nodeDataRegistry->get(nodeCast);
        
        // Je ne dois pas faire que get child car il peux y avoir nodeIf nodeWhile
        const auto& childs = (nodeData.*getter)();
        REQUIRE(childs.size() == sizeof...(Matchers));
        verifierChilds(childs, std::index_sequence_for<Matchers...>{});
    }
};


// Faire les helper pour éviter d'avoir beaucoup de syntaxe dans les tests template 
auto Literal(const std::string& valeur, EnvironnementAST& env) {
    return matcherNode<&LiteralNodeData::getToken, NodeLiteral, std::string, &Token::value>(env, valeur);
}

// Helper pour les opérations
auto operateur(EnvironnementAST& env) {
    return [&env](const std::string& type, auto gauche, auto droite) {
        return matcherGeneralBinaire<NodeOperation, &OperationNodeData::getToken, std::string, &Token::value, decltype(gauche), decltype(droite)>(
            env, type, gauche, droite
        );
    };
}

// Helper pour les listes d'child
template<typename T, auto Method, typename M>
struct matcherPropriete {
    M m;
    EnvironnementAST& env;

    matcherPropriete(EnvironnementAST& env, M m) : env(env), m(std::move(m)) {}


    void operator()(INode* n) {
        auto* t = verifierTypeEtCaster<T>(n);
        auto& nodeData = env.nodeDataRegistry->get(t);

        m((nodeData.*Method)()); 
    }
};

// matcher utilitaire pour appliquer plusieurs matcher sur le même node
template<typename T, typename... Ms>
struct matcherCombine {
    std::tuple<Ms...> ms;
    EnvironnementAST& env;

    matcherCombine(EnvironnementAST& env, Ms... ms) : env(env), ms(std::make_tuple(std::move(ms)...)) {}

    void operator()(INode* n) {
        auto* t = verifierTypeEtCaster<T>(n);
        // Applique chaque matcher au nœud casté
        std::apply([&](auto&&... args) { (args(t), ...); }, ms);
    }
};

// C'est un fichier de test exclusif à l'tree syntaxique abstrait première partie du test sera de vérifier la construction de l'tree 
// d'equation et de tester des operations de base comme l'addition, la soustraction, la multiplication et la division.

TEST_CASE("Construction Tree Equation Simple", "[AST]")
{
    cout<< "\nTest 1 - Building a tree for the equation '1 + 10 * 50'" << endl;
    
    EnvironnementAST env;
    INode* tree = construireEquationDepuisString(env, "1 + 10 * 50");

    auto verificateur = 
    operateur(env)(
        "+", 
        Literal("1", env), 
        operateur(env)(
            "*", 
            Literal("10", env), 
            Literal("50", env)
        ));
    
    verificateur(tree);
}

TEST_CASE("Builder Tree equation priorite", "[AST]")
{
    cout << "\nTest 2 - Building a tree with Prysma rules" << endl;
    EnvironnementAST env;
    INode* tree = construireEquationDepuisString(env, "40 / 2 + 10 - 5 * 3");
    
    // TODO: il faudrait faire une factory ou un builder spécifique qui injecte le registre automatiquement (env ici)

    auto verificateur = 
    operateur(env)(
        "+", 
        operateur(env)(
            "/", 
            Literal("40", env), 
            Literal("2", env)
        ), 
        operateur(env)(
            "-", 
            Literal("10", env), 
            operateur(env)(
                "*", 
                Literal("5", env), 
                Literal("3", env)
            )
        )
    );
    verificateur(tree);
}

TEST_CASE("Builder Tree equation depth parenthèse", "[AST]")
{
    cout << "\nTest 3 - Building a tree with parentheses" << endl;
    EnvironnementAST env;
    INode* tree = construireEquationDepuisString(env, "(((40/2 +10)+ 5 * 3)+10)");

    auto verificateur = 
    operateur(env)(
        "+", 
        operateur(env)(
            "+", 
            operateur(env)(
                "+", 
                operateur(env)(
                    "/", 
                    Literal("40", env), 
                    Literal("2", env)
                ), 
                Literal("10", env)
            ), 
            operateur(env)(
                "*", 
                Literal("5", env), 
                Literal("3", env)
            )
        ), 
        Literal("10", env)
    );

    verificateur(tree);
}

// Début des tests pour l'tree d'instruction 

// Test de branchement if simple avec else
TEST_CASE("Construction Tree If simple avec else", "[AST][Branch]")
{
    cout << "\nTest 4 - Simple if/else tree" << endl;
    EnvironnementAST env;

    // Code Prysma : if avec condition et deux branches
    INode* tree = construireTreeDepuisString(env,
        "if (a > 5) { aff x = 1; } else { aff x = 2; }"
    );
     
    REQUIRE(tree != nullptr);

    // 1. Racine = NodeInstruction qui contient le if
    auto* racine = dynamic_cast<NodeInstruction*>(tree);
    REQUIRE(racine != nullptr);

    auto racineData = env.nodeDataRegistry->get(racine);
    REQUIRE(racineData.getChildren().size() == 1);


    // 2. Premier child = NodeIf
    auto* nodeIf = dynamic_cast<NodeIf*>(racineData.getChildren()[0]);
    REQUIRE(nodeIf != nullptr);

    // 3. Condition pas nulle
    auto& nodeIfData = env.nodeDataRegistry->get(nodeIf);
    REQUIRE(nodeIfData.getNodeCondition() != nullptr);

    // 4. Vérifier condition : opération '>'
    auto* condition = dynamic_cast<NodeOperation*>(nodeIfData.getNodeCondition());
    REQUIRE(condition != nullptr);

    auto conditionData = env.nodeDataRegistry->get(condition);
    REQUIRE(conditionData.getToken().type == TOKEN_GREATER);

    // 5. Bloc if existe et contient 1 instruction
    auto* blocIf = dynamic_cast<NodeInstruction*>(nodeIfData.getNodeBlocIf());
    REQUIRE(blocIf != nullptr);

    auto blocIfData = env.nodeDataRegistry->get(blocIf);
    REQUIRE(blocIfData.getChildren().size() == 1);

    // 6. Bloc else existe et contient 1 instruction
    auto* blocElse = dynamic_cast<NodeInstruction*>(nodeIfData.getNodeBlocElse());
    REQUIRE(blocElse != nullptr);

    auto blocElseData = env.nodeDataRegistry->get(blocElse);
    REQUIRE(blocElseData.getChildren().size() == 1);

    // 7. Bloc endif existe (node de output)
    REQUIRE(nodeIfData.getNodeBlocEndif() != nullptr);
}

// Test if sans else
TEST_CASE("Construction Tree If sans else", "[AST][Branch]")
{
    cout << "\nTest 5 - If tree without else" << endl;
    EnvironnementAST env;

    INode* tree = construireTreeDepuisString(env,
        "if (a == 10) { aff y = 42; }"
    );

    REQUIRE(tree != nullptr);

    auto* racine = dynamic_cast<NodeInstruction*>(tree);
    REQUIRE(racine != nullptr);

    auto racineData = env.nodeDataRegistry->get(racine);

    // 1. NodeIf
    auto* nodeIf = dynamic_cast<NodeIf*>(racineData.getChildren()[0]);
    REQUIRE(nodeIf != nullptr);

    auto& nodeIfData = env.nodeDataRegistry->get(nodeIf);

    // 2. Condition ==
    auto* condition = dynamic_cast<NodeOperation*>(nodeIfData.getNodeCondition());
    REQUIRE(condition != nullptr);

    auto conditionData = env.nodeDataRegistry->get(condition);
    REQUIRE(conditionData.getToken().type == TOKEN_EQUAL_EQUAL);

    // 3. Bloc if a 1 child
    auto* blocIf = dynamic_cast<NodeInstruction*>(nodeIfData.getNodeBlocIf());
    REQUIRE(blocIf != nullptr);

    auto blocIfData = env.nodeDataRegistry->get(blocIf);
    REQUIRE(blocIfData.getChildren().size() == 1);

    // 4. Pas de else
    REQUIRE(nodeIfData.getNodeBlocElse() == nullptr);

    // 5. Endif existe quand même
    REQUIRE(nodeIfData.getNodeBlocEndif() != nullptr);
}

// Test if avec condition logique && 
TEST_CASE("Construction Tree If condition logique ET", "[AST][Branch]")
{
    cout << "\nTest 6 - If tree with &&" << endl;
    EnvironnementAST env;

    INode* tree = construireTreeDepuisString(env,
        "if (a > 1 && b < 10) { aff z = 0; }"
    );

    REQUIRE(tree != nullptr);

    auto* racine = dynamic_cast<NodeInstruction*>(tree);
    REQUIRE(racine != nullptr);

    auto racineData = env.nodeDataRegistry->get(racine);
    REQUIRE(racineData.getChildren().size() == 1);

    auto* nodeIf = dynamic_cast<NodeIf*>(racineData.getChildren()[0]);
    REQUIRE(nodeIf != nullptr);

    auto& nodeIfData = env.nodeDataRegistry->get(nodeIf);

    // Condition racine = &&
    auto* condition = dynamic_cast<NodeOperation*>(nodeIfData.getNodeCondition());
    REQUIRE(condition != nullptr);

    auto conditionData = env.nodeDataRegistry->get(condition);
    REQUIRE(conditionData.getToken().type == TOKEN_AND);

    // Gauche du && = '>'
    auto* gauche = dynamic_cast<NodeOperation*>(conditionData.getLeft());
    REQUIRE(gauche != nullptr);

    auto gaucheData = env.nodeDataRegistry->get(gauche);
    REQUIRE(gaucheData.getToken().type == TOKEN_GREATER);

    // Droite du && = '<'
    auto* droite = dynamic_cast<NodeOperation*>(conditionData.getRight());
    REQUIRE(droite != nullptr);

    auto droiteData = env.nodeDataRegistry->get(droite);
    REQUIRE(droiteData.getToken().type == TOKEN_LESS);
}

// Test while simple
TEST_CASE("Construction Tree While simple", "[AST][Branch]")
{
    cout << "\nTest 7 - Simple while tree" << endl;
    EnvironnementAST env;

    INode* tree = construireTreeDepuisString(env,
        "while (i < 10) { aff i = i + 1; }"
    );

    REQUIRE(tree != nullptr);

    auto* racine = dynamic_cast<NodeInstruction*>(tree);
    REQUIRE(racine != nullptr);

    auto racineData = env.nodeDataRegistry->get(racine);
    REQUIRE(racineData.getChildren().size() == 1);

    // 1. NodeWhile
    auto* nodeWhile = dynamic_cast<NodeWhile*>(racineData.getChildren()[0]);
    REQUIRE(nodeWhile != nullptr);

    auto& nodeWhileData = env.nodeDataRegistry->get(nodeWhile);

    // 2. Condition '<'
    auto* condition = dynamic_cast<NodeOperation*>(nodeWhileData.getNodeCondition());
    REQUIRE(condition != nullptr);

    auto conditionData = env.nodeDataRegistry->get(condition);
    REQUIRE(conditionData.getToken().type == TOKEN_LESS);

    // 3. Bloc while a 1 instruction
    auto* blocWhile = dynamic_cast<NodeInstruction*>(nodeWhileData.getWhileBlock());
    REQUIRE(blocWhile != nullptr);

    auto blocWhileData = env.nodeDataRegistry->get(blocWhile);
    REQUIRE(blocWhileData.getChildren().size() == 1);

    // 4. Bloc fin while existe
    REQUIRE(nodeWhileData.getWhileEndBlock() != nullptr);
}

// Test while avec condition logique ||
TEST_CASE("Construction Tree While condition OU", "[AST][Branch]")
{
    cout << "\nTest 8 - While tree with ||" << endl;
    EnvironnementAST env;

    INode* tree = construireTreeDepuisString(env,
        "while (x == 0 || y == 0) { aff x = 1; }"
    );

    REQUIRE(tree != nullptr);

    auto* racine = dynamic_cast<NodeInstruction*>(tree);
    REQUIRE(racine != nullptr);

    auto racineData = env.nodeDataRegistry->get(racine);
    REQUIRE(racineData.getChildren().size() == 1);

    auto* nodeWhile = dynamic_cast<NodeWhile*>(racineData.getChildren()[0]);
    REQUIRE(nodeWhile != nullptr);

    auto& nodeWhileData = env.nodeDataRegistry->get(nodeWhile);

    // Condition racine = ||
    auto* condition = dynamic_cast<NodeOperation*>(nodeWhileData.getNodeCondition());
    REQUIRE(condition != nullptr);

    auto conditionData = env.nodeDataRegistry->get(condition);
    REQUIRE(conditionData.getToken().type == TOKEN_OR);

    // Gauche du || = '=='
    auto* gauche = dynamic_cast<NodeOperation*>(conditionData.getLeft());
    REQUIRE(gauche != nullptr);

    auto gaucheData = env.nodeDataRegistry->get(gauche);
    REQUIRE(gaucheData.getToken().type == TOKEN_EQUAL_EQUAL);

    // Droite du || = '=='
    auto* droite = dynamic_cast<NodeOperation*>(conditionData.getRight());
    REQUIRE(droite != nullptr);

    auto droiteData = env.nodeDataRegistry->get(droite);
    REQUIRE(droiteData.getToken().type == TOKEN_EQUAL_EQUAL);
}

// Test while avec plusieurs instructions dans le body
TEST_CASE("Construction Tree While plusieurs instructions", "[AST][Branch]")
{
    cout << "\nTest 9 - While tree with multiple instructions" << endl;
    EnvironnementAST env;

    INode* tree = construireTreeDepuisString(env,
        "while (i < 5) { aff a = 0; aff i = i + 1; }"
    );

    REQUIRE(tree != nullptr);

    auto* racine = dynamic_cast<NodeInstruction*>(tree);
    REQUIRE(racine != nullptr);

    auto racineData = env.nodeDataRegistry->get(racine);
    REQUIRE(racineData.getChildren().size() == 1);

    auto* nodeWhile = dynamic_cast<NodeWhile*>(racineData.getChildren()[0]);
    REQUIRE(nodeWhile != nullptr);

    auto& nodeWhileData = env.nodeDataRegistry->get(nodeWhile);

    // Bloc while contient 2 instructions (aff + aff)
    auto* blocWhile = dynamic_cast<NodeInstruction*>(nodeWhileData.getWhileBlock());
    REQUIRE(blocWhile != nullptr);

    auto blocWhileData = env.nodeDataRegistry->get(blocWhile);
    REQUIRE(blocWhileData.getChildren().size() == 2);
}
