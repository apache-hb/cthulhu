#include "token.hpp"

namespace cthulhu {
    Range::Range(Lexer* lexer)
        : lexer(lexer)
        , offset(0)
        , line(0)
        , column(0)
        , length(0)
    { }

    bool Token::is(Token::Type other) const { return type == other; }

    Token::Data::Data() {

    }

    Token::Data::~Data() {

    }
}
