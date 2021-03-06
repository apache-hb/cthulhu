#pragma once

#include <parser.hpp>
#include "tlexer.hpp"

using namespace ast;

struct TestParser : Parser {
    TestParser(TestLexer* lexer) : Parser(lexer), lex(lexer) { }

    void finish() { lex->expect(Token::END); }

    template<typename F>
    void expect(F&& func, ptr<ast::Node> node) {
        ptr<ast::Node> temp = func();
        ASSERT(temp);
        ASSERT(node);
        ASSERT(temp->equals(node));
    }

private:
    TestLexer* lex;
};
