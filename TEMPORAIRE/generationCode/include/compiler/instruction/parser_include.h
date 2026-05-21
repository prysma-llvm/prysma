#ifndef PARSEUR_INCLUDE_H
#define PARSEUR_INCLUDE_H

#include "compiler/ast/registry/context_parser.h"
#include "compiler/lexer/lexer.h"
#include "compiler/parser/interfaces/i_parser.h"
#include "compiler/parser/parser_base.h"
#include <cstddef>


class ParserInclude : public IParser, public ParserBase
{
private:
    ContextParser& _contextParser;
    
public: 

    ParserInclude(ContextParser& contextParser);
    ~ParserInclude();
    
    auto parse(std::vector<Token>& tokens, std::size_t& index) -> INode* override;
};


#endif /* PARSEUR_INCLUDE_H */




