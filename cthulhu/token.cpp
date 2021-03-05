#include "token.hpp"
#include "error.hpp"

#include <fmt/core.h>

namespace cthulhu {
    Range::Range() : Range(nullptr) { }

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

    bool Token::is(Token::Type other) const { 
        return type == other; 
    }

    Number::Number(size_t num, const utf8::string* suf) 
        : number(num)
        , suffix(suf)
    { }

    Token::Token() 
        : type(Token::INVALID)
        , data({}) 
    { }

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
        case Token::STRING: return data.string == other.data.string;
        default: throw std::runtime_error("unreachable");
        }
    }

    bool Token::operator!=(const Token& other) const {
        if (type != other.type) {
            return true;
        }

        switch (type) {
        case Token::IDENT: return data.ident != other.data.ident;
        case Token::STRING: return data.string != other.data.string;
        default: throw std::runtime_error("unreachable");
        }
    }

#define CHECK_TYPE(type) if (!is(type)) { throw LexerError(where.lexer, LexerError::CAST); }

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

    c32 Token::letter() const {
        CHECK_TYPE(Token::CHAR);

        return data.letter;
    }

    Number Token::number() const {
        CHECK_TYPE(Token::INT);
        
        return data.digit;
    }

    bool Token::valid() const {
        return type != Token::INVALID;
    }
}
