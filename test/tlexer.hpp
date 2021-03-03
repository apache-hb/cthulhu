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

    Token expectKey(Key key) {
        auto token = expect(Token::KEY);
        ASSERT(token.key() == key);
        return token;
    }
};
