#include "token.h"
#include "lexer.h"

#include <fmt/format.h>

std::string Token::repr() {
    auto body = fmt::format("{}:{} ", range.offset, range.length);
    switch (type) {
    case Token::IDENT: return body + "ident";
    case Token::KEY: return body + "key";
    case Token::STRING: return body + "string";
    case Token::CHAR: return body + "char";
    case Token::INT: return body + "int";
    case Token::END: return body + "eof";
    default: return body + "invalid";
    }
}

std::string Token::text() {
    return range.lexer->text.substr(range.offset, range.length);
}
