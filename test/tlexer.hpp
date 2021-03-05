#pragma once

#include <lexer.hpp>
#include "test.hpp"

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

    Token expectIdent(const utf8::string& id) {
        auto token = expect(Token::IDENT);
        ASSERT(*token.ident() == id);
        return token;
    }

    Token expectString(const utf8::string& str) {
        auto token = expect(Token::STRING);
        ASSERT(*token.string() == str);
        return token;
    }

    Token expectChar(c32 c) {
        auto token = expect(Token::CHAR);
        fprintf(stderr, "%d %d\n", (int)token.letter(), (int)c);
        ASSERT(token.letter() == c);
        return token;
    }

    Token expectKey(Key key) {
        auto token = expect(Token::KEY);
        ASSERT(token.key() == key);
        return token;
    }

    Token expectNumber(size_t num, const utf8::string& suffix = "") {
        auto token = expect(Token::INT);
        auto data = token.number();
        fprintf(stderr, "%zu %zu\n", data.number, num);
        ASSERT(data.number == num);
        if (!suffix.empty()) {
            ASSERT(*data.suffix == suffix);
        }
        return token;
    }
};
