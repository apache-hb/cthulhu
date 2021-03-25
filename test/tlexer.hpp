#pragma once

#include "test.hpp"
#include <lexer.hpp>

using namespace cthulhu;

struct TestLexer : Lexer {
    TestLexer(StreamHandle* handle)
        : Lexer(Stream(handle), "test") 
    { }

    Token expect(Token::Type type) {
        auto token = read();
        ASSERT(token.is(type));
        return token;
    }

    Token expectIdent(const str& id) {
        auto token = expect(Token::IDENT);
        ASSERT(*token.ident() == id);
        return token;
    }

    Token expectString(const str& str) {
        auto token = expect(Token::STRING);
        ASSERT(*token.string() == str);
        return token;
    }

    Token expectChar(char32_t c) {
        auto token = expect(Token::CHAR);
        ASSERT(token.letter() == c);
        return token;
    }

    Token expectKey(Key key) {
        auto token = expect(Token::KEY);
        ASSERT(token.key() == key);
        return token;
    }

    Token expectNumber(size_t num, const str& suffix = "") {
        auto token = expect(Token::INT);
        auto data = token.number();
        ASSERT(data.number == num);
        if (!suffix.empty()) {
            ASSERT(*data.suffix == suffix);
        }
        return token;
    }

    Token ident(const str& id) {
        return Token(Token::IDENT, { .ident = idents->intern(id) });
    }

    const str* string(const str& str) {
        return strings->intern(str);
    }
};
