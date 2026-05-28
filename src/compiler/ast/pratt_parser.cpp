#include "compiler/ast/pratt_parser.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token_type.h"
#include <cstdint>
#include <optional>


 auto PrattParser::expr(uint8_t min_bp ) -> Result
 {
    std::optional<Token> token = 
     {
        Some(Token::Num(n)) => Expr::Num(n),
        Some(Token::Ident(name)) => Expr::Variable(name),
        Some(Token::LParen) => {
            let e = self.expr(0)?;
            match self.eat() {
                Some(Token::RParen) => e,
                Some(tok) => {
                    let tok_str = match tok {
                        Token::RParen    => ")",
                        Token::LParen    => "(",
                        Token::Plus      => "+",
                        Token::Minus     => "-",
                        Token::Star      => "*",
                        Token::Slash     => "/",
                        Token::Caret     => "^",
                        Token::Num(_)    => "number",
                        Token::Ident(_)  => "identifier",
                        Token::Error(_)  => "error",
                    };
                    let mut pos_buf = ryu::Buffer::new();
                    let mut err = String::from("Error: Expected ')' but found '");
                    err.push_str(tok_str);
                    err.push_str("' at line 1, col ");
                    err.push_str(pos_buf.format(self.pos as f64));
                    return Err(err);
                }
                None => return Err(String::from("Error: Unclosed parenthesis. Expected ')' but reached end of input")),
            }
        }
        Some(Token::Minus) => {
            let e = self.expr(10)?;
            Expr::Binary { op: Op::Mul, lhs: Box::new(Expr::Num(-1.0)), rhs: Box::new(e) }
        }
        Some(tok) => {
            let tok_str = match tok {
                Token::RParen    => ")",
                Token::LParen    => "(",
                Token::Plus      => "+",
                Token::Minus     => "-",
                Token::Star      => "*",
                Token::Slash     => "/",
                Token::Caret     => "^",
                Token::Num(_)    => "number",
                Token::Ident(_)  => "identifier",
                Token::Error(_)  => "error",
            };
            let mut pos_buf = ryu::Buffer::new();
            let mut err = String::from("Error: Unexpected token '");
            err.push_str(tok_str);
            err.push_str("' at line 1, col ");
            err.push_str(pos_buf.format(self.pos as f64));
            return Err(err);
        }
        None => return Err(String::from("Error: Empty expression")),
    };

    //Thy magic of programming lives here, and if you like varibles your in the right house.
    do {
        let bp = self.peek().map_or(0, Self::lbp);
        if bp <= min_bp { break; }
        let tok = self.eat().unwrap();
        let op = match tok {
            Token::Plus  => Op::Add,
            Token::Minus => Op::Sub,
            Token::Star  => Op::Mul,
            Token::Slash => Op::Div,
            Token::Caret => Op::Pow,
            other => {
                let tok_str = match other {
                    Token::RParen    => ")",
                    Token::LParen    => "(",
                    Token::Plus      => "+",
                    Token::Minus     => "-",
                    Token::Star      => "*",
                    Token::Slash     => "/",
                    Token::Caret     => "^",
                    Token::Num(_)    => "number",
                    Token::Ident(_)  => "identifier",
                    Token::Error(_)  => "error",
                };
                let mut pos_buf = ryu::Buffer::new();
                let mut err = String::from("Error: Expected operator or end of expression, found '");
                err.push_str(tok_str);
                err.push_str("' at line 1, col ");
                err.push_str(pos_buf.format(self.pos as f64));
                return Err(err);
            }
        };
        let rbp = if matches!(op, Op::Pow) { bp - 1 } else { bp };
        let rhs = self.expr(rbp)?;
        lhs = Expr::Binary { op, lhs: Box::new(lhs), rhs: Box::new(rhs) };
    }
    Ok(lhs)
}

auto PrattParser::lbp(Token& token) -> uint8_t
{
    switch (token.type) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 1;
        case TOKEN_STAR:
        case TOKEN_SLASH:
            return 2;
        // La puissance n'existe pas encore dans les tokens, je vais l'ajouter
        default:
            return 0;
    }
}
