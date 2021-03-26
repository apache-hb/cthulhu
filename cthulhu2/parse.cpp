#include "parse.h"

Parser::Parser(Lexer* source)
    : source(source)
{ }

Node* Parser::include() {
    if (!eat(USING)) {
        return nullptr;
    }

    auto path = idents(COLON2);

    Import* out = new Import();
    out->path = path;
    return out;
}

Unit* Parser::unit() {
    Unit* out = new Unit();

    while (true) {
        auto* inc = include();

        if (inc == nullptr) {
            break;
        } else {
            out->imports.push_back((Import*)inc);
        }
    }

    return out;
}

std::vector<Ident> Parser::idents(Key sep) {
    std::vector<Ident> tokens;
    do { tokens.push_back(expect(Token::IDENT).data.ident); } while (eat(sep));
    return tokens;
}

bool Parser::eat(Key key) {
    auto tok = peek();
    if (tok.type == Token::KEY && tok.data.key == key) {
        next();
        return true;
    }

    return false;
}

Token Parser::expect(Token::Type type) {
    auto tok = next();
    if (tok.type == type) {
        return tok;
    }

    throw Error(Error::UNEXPECTED_TYPE, tok, { .type = type });
}

Token Parser::next() {
    auto temp = peek();
    ahead = {};
    return temp;
}

Token Parser::peek() {
    if (ahead.type == Token::MONOSTATE) {
        ahead = source->read();
    }

    return ahead;
}
