#include "token.hpp"
#include "error.hpp"

#include <fmt/core.h>

namespace cthulhu {
    Range::Range(Lexer* lexer)
        : lexer(lexer)
        , offset(0)
        , line(0)
        , column(0)
        , length(0)
    { }

    Range::Range(Lexer* lexer, size_t offset, size_t line, size_t column, size_t length) 
        : lexer(lexer)
        , offset(offset)
        , line(line)
        , column(column)
        , length(length)
    { }

    Range Range::to(const Range& end) const {
        return Range(lexer, offset, line, column, end.offset - offset);
    }

    Lexer* Range::source() const {
        return lexer;
    }

    bool Token::is(Token::Type other) const { 
        return type == other; 
    }

    Token::Token(Range where, Type type, TokenData data) 
        : where(where)
        , type(type)
        , data(data)
    { }

    bool Token::operator==(const Token& other) const {
        if (type != other.type) {
            return false;
        }

        switch (type) {
        case Token::IDENT: return data.ident == other.data.ident;
        default: throw std::runtime_error("unreachable");
        }
    }

    bool Token::operator!=(const Token& other) const {
        if (type != other.type) {
            return true;
        }

        switch (type) {
        case Token::IDENT: return data.ident != other.data.ident;
        default: throw std::runtime_error("unreachable");
        }
    }

#define CHECK_TYPE(type) if (!is(type)) { throw LexerError(where.source(), LexerError::CAST); }

    Key Token::key() const {
        CHECK_TYPE(Token::KEY);

        return data.key;
    }

    const utf8::string* Token::ident() const {
        CHECK_TYPE(Token::IDENT);

        return data.ident;
    }

    const utf8::string* Token::string() const {
        CHECK_TYPE(Token::STRING);

        return data.string;
    }
}
