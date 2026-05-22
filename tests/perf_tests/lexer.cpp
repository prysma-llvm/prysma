#include <cstddef>
#include <cstdint>
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.hpp"
#include "compiler/lexer/lexer.h"
#include <string>
#include <vector>
#include <random>
#include <sys/prctl.h>

// Disable perf counters as early as possible (before main).
// perf stat starts with enable_on_exec=1, so counters are active from exec().
// This constructor fires before Catch2's main(), killing startup noise.
// Each TEST_CASE then re-enables counters only around its hot loop.
__attribute__((constructor))
static void disablePerfCountersAtStartup() {
    prctl(PR_TASK_PERF_EVENTS_DISABLE, 0, 0, 0, 0);
}

// Shared deterministic seed for generating test data
constexpr uint64_t DEFAULT_SEED = 12345ULL;

static std::string generateStressCode(uint64_t seed, size_t numTokens) {
    std::mt19937_64 rng(seed);

    const std::vector<std::string> keywords = {
        "fn", "if", "for", "dec", "aff", "ref", "arg", "new", "ptr",
        "else", "true", "void", "bool", "char", "call", "pass",
        "while", "false", "scope", "unref", "int64", "int32", "float", "class",
        "return", "string", "delete", "public", "include", "private", "protected"
    };

    const std::vector<std::string> operators = {
        "+", "-", "*", "/", "%", "=", "==", "!=", "<", ">", "<=", ">=",
        "&&", "||", "!", "&", "|"
    };

    const std::vector<std::string> punctuation = {
        "(", ")", "{", "}", "[", "]", ";", ",", ":", "."
    };

    const std::vector<std::string> identifiers = {
        "x", "y", "z", "count", "value", "result", "temp", "idx",
        "data", "buf", "len", "max", "min", "ptr", "node", "item"
    };

    const std::vector<std::string> literals = {
        "0", "1", "42", "100", "255", "1024", "99999",
        "3.14", "0.001", "2.718", "100.0",
        "\"hello\"", "\"world\"", "\"test\""
    };

    enum Category { KW, OP, PUNCT, IDENT, LIT, NUM_CAT };

    std::string code;
    code.reserve(numTokens * 8);

    for (size_t i = 0; i < numTokens; ++i) {
        auto cat = static_cast<Category>(rng() % NUM_CAT);
        switch (cat) {
            case KW:    code += keywords[rng() % keywords.size()]; break;
            case OP:    code += operators[rng() % operators.size()]; break;
            case PUNCT: code += punctuation[rng() % punctuation.size()]; break;
            case IDENT: code += identifiers[rng() % identifiers.size()]; break;
            case LIT:   code += literals[rng() % literals.size()]; break;
            default:    break;
        }
        code += (rng() % 10 == 0) ? '\n' : ' ';
    }

    return code;
}

static std::string generateKeywordsHeavyCode(uint64_t seed, size_t numTokens) {
    std::mt19937_64 rng(seed);
    const std::vector<std::string> keywords = {
        "fn", "if", "for", "dec", "aff", "ref", "arg", "new", "ptr",
        "else", "true", "void", "bool", "char", "call", "pass",
        "while", "false", "scope", "unref", "int64", "int32", "float", "class",
        "return", "string", "delete", "public", "include", "private", "protected"
    };
    std::string code;
    code.reserve(numTokens * 8);
    for (size_t i = 0; i < numTokens; ++i) {
        code += keywords[rng() % keywords.size()];
        code += (rng() % 10 == 0) ? '\n' : ' ';
    }
    return code;
}

static std::string generateNumericLiteralsCode(uint64_t seed, size_t numTokens) {
    std::mt19937_64 rng(seed);
    const std::vector<std::string> literals = {
        "0", "1", "42", "100", "255", "1024", "99999",
        "3.14", "0.001", "2.718", "100.0"
    };
    std::string code;
    code.reserve(numTokens * 8);
    for (size_t i = 0; i < numTokens; ++i) {
        code += literals[rng() % literals.size()];
        code += (rng() % 10 == 0) ? '\n' : ' ';
    }
    return code;
}

TEST_CASE("Lexer DFA - Fused Tokens 50k", "[lexer][pmu]") {
    constexpr size_t NUM_TOKENS = 50000;
    const std::string stress = generateStressCode(DEFAULT_SEED, NUM_TOKENS);

    // Enable perf counters only around the hot loop
    prctl(PR_TASK_PERF_EVENTS_ENABLE, 0, 0, 0, 0);

    for (int i = 0; i < 1000; ++i) {
        auto tokens = Lexer::tokenize(stress);
        (void)tokens;
    }

    // Disable perf counters — only the hot loop above is measured
    prctl(PR_TASK_PERF_EVENTS_DISABLE, 0, 0, 0, 0);
}

TEST_CASE("Lexer DFA - Keywords Heavy", "[lexer][pmu]") {
    constexpr size_t NUM_TOKENS = 50000;
    const std::string stress = generateKeywordsHeavyCode(DEFAULT_SEED, NUM_TOKENS);

    prctl(PR_TASK_PERF_EVENTS_ENABLE, 0, 0, 0, 0);

    for (int i = 0; i < 1000; ++i) {
        auto tokens = Lexer::tokenize(stress);
        (void)tokens;
    }

    prctl(PR_TASK_PERF_EVENTS_DISABLE, 0, 0, 0, 0);
}

TEST_CASE("Lexer DFA - Numeric Literals", "[lexer][pmu]") {
    constexpr size_t NUM_TOKENS = 50000;
    const std::string stress = generateNumericLiteralsCode(DEFAULT_SEED, NUM_TOKENS);

    prctl(PR_TASK_PERF_EVENTS_ENABLE, 0, 0, 0, 0);

    for (int i = 0; i < 1000; ++i) {
        auto tokens = Lexer::tokenize(stress);
        (void)tokens;
    }

    prctl(PR_TASK_PERF_EVENTS_DISABLE, 0, 0, 0, 0);
}
