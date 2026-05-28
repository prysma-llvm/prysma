#ifndef DB555C9B_48B0_4430_8E48_A8A560EE956C
#define DB555C9B_48B0_4430_8E48_A8A560EE956C

#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser_base.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <llvm-18/llvm/ADT/StringRef.h>
#include <string>
#include <variant>
#include <vector>


// The official Pratt parser library is in this folder: lib/core/pratt_parser/src/lib.rs
// The library has been translated to C++ and adapted for the PrySma compiler

enum class Op : uint8_t {Add, Sub, Mul, Div, Pow };

struct Expr;

struct Num{
    double value;
};

struct Variable
{
    llvm::StringRef name;
};

struct Binary
{
    Op op; 
    Expr* lhs; 
    Expr* rhs;
};

struct Expr : std::variant<Num, Variable, Binary> {
    using std::variant<Num, Variable, Binary>::variant;
};

using Result = std::variant<Expr, std::string>;


class PrattParser
{
private:
    const std::vector<Token>* _tokens; 
    std::size_t* _index;
    ParserBase* _parserBase; // TODO à surveiller

private:

    auto peak() -> const Token& { return (*_tokens)[*_index]; }

     //Heres my prioritys again, kind of, but hey it maps it to the proper ones.
    static auto lbp(Token& token) -> uint8_t;

public:

    explicit PrattParser(
        const std::vector<Token>* tokens,
        std::size_t* index,
        ParserBase* parserBase
        )
        : 
        _tokens(tokens),
        _index(index),
        _parserBase(parserBase)
        {
            // Uniquement en mode debug et non en mode release
            assert(_tokens != nullptr);
            assert(_index != nullptr);
            assert(_parserBase != nullptr);
        };

    static auto expr(uint8_t min_bp ) -> Result;

};

#endif /* DB555C9B_48B0_4430_8E48_A8A560EE956C */
