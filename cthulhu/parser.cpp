#include "parser.hpp"
#include "lexer.hpp"

#define START(key) if (!eatKey(Key::key)) { return nullptr; }
#define TRY(expr) if (auto it = (expr); it) { return it; }

namespace {
    using namespace cthulhu;

    enum BinaryPrecedence : int {
        P_NONE = 0,

        P_TERNARY,
        P_LOGIC,
        P_EQUALITY,
        P_COMPARE,
        P_BITWISE,
        P_BITSHIFT,
        P_MATH,
        P_MUL
    };

    int precedence(Token tok) {
        if (!tok.is(Token::KEY)) {
            return P_NONE;
        }

        switch (tok.key()) {
        case Key::QUESTION:
            return P_TERNARY;
        case Key::AND: case Key::OR:
            return P_LOGIC;
        case Key::EQ: case Key::NEQ:
            return P_EQUALITY;
        case Key::GT: case Key::GTE: case Key::LT: case Key::LTE:
            return P_COMPARE;
        case Key::XOR: case Key::BITAND: case Key::BITOR:
            return P_BITWISE;
        case Key::SHL: case Key::SHR:
            return P_BITSHIFT;
        case Key::ADD: case Key::SUB:
            return P_MATH;
        case Key::MOD: case Key::MUL: case Key::DIV:
            return P_MUL;
        default:
            return P_NONE;
        }
    }

    int associate(int other) {
        return other != P_LOGIC;
    }

    bool validUnary(Key key) {
        return key == Key::ADD 
            || key == Key::SUB 
            || key == Key::MUL 
            || key == Key::BITAND 
            || key == Key::NOT
            || key == Key::FLIP;
    }
}

namespace cthulhu {
    Parser::Parser(Lexer* lexer)
        : lexer(lexer)
    { }

    //
    // type parsing
    //

    ptr<ast::Type> Parser::parseType() {
        // try and match all prefixes
        TRY(parsePointerType());
        TRY(parseMutableType());
        TRY(parseArrayType());
        TRY(parseClosureType());

        // if nothing matched then this must be a qualified type
        return parseQualifiedType();
    }

    ptr<ast::PointerType> Parser::parsePointerType() {
        START(MUL);

        return MAKE<ast::PointerType>(parseType());
    }

    ptr<ast::MutableType> Parser::parseMutableType() {
        START(VAR);

        return MAKE<ast::MutableType>(parseType());
    }

    ptr<ast::ArrayType> Parser::parseArrayType() {
        START(LSQUARE);

        auto type = parseType();
        auto size = eatKey(Key::COLON) ? parseExpr() : nullptr;

        expect(Token::KEY, Key::RSQUARE);

        return MAKE<ast::ArrayType>(type, size);
    }

    ptr<ast::ClosureType> Parser::parseClosureType() {
        START(LPAREN);

        auto args = collect<ast::Type>(Key::COMMA, [&] { return parseType(); }, false);

        expect(Token::KEY, Key::RPAREN);
        expect(Token::KEY, Key::ARROW);

        auto result = parseType();

        return MAKE<ast::ClosureType>(args, result);
    }

    ptr<ast::NameType> Parser::parseNameType() {
        if (auto name = eat(Token::IDENT); name) {
            return MAKE<ast::NameType>(MAKE<ast::Ident>(name));
        } else {
            return nullptr;
        }
    }

    ptr<ast::QualifiedType> Parser::parseQualifiedType() {
        auto names = collect<ast::NameType>(Key::COLON2, [&] { return parseNameType(); }, false);
        
        if (!names.empty()) {
            return MAKE<ast::QualifiedType>(names);
        } else {
            return nullptr;
        }
    }


    //
    // expression parsing
    //

    ptr<ast::Expr> Parser::parseExpr() {
        return parseBinaryExpr(P_LOGIC);
    }

    ptr<ast::Expr> Parser::parseBinaryExpr(int mprec) {
        auto lhs = parsePrimaryExpr();

        while (true) {
            Token token = next();
            auto nprec = precedence(token);
            if (nprec == P_NONE || nprec < mprec) {
                ahead = token;
                break;
            }

            if (nprec == P_TERNARY) {
                auto rhs = parseExpr();
                expect(Token::KEY, Key::COLON);
                lhs = MAKE<ast::TernaryExpr>(lhs, rhs, parseExpr());
            } else {
                auto rhs = parseBinaryExpr(nprec + associate(nprec));
                lhs = MAKE<ast::BinaryExpr>(token, lhs, rhs);
            }
        }
        return lhs;
    }

    ptr<ast::Expr> Parser::parsePrimaryExpr() {
        Token token = next();

        ptr<ast::Expr> expr;

        if (token.is(Token::KEY)) {
            if (validUnary(token.key())) {
                expr = MAKE<ast::UnaryExpr>(token, parsePrimaryExpr());
            } else if (token.key() == Key::LPAREN) {
                expr = parseExpr();
                expect(Token::KEY, Key::RPAREN);
            } else if (token.key() == Key::COERCE) {
                expect(Token::KEY, Key::BEGIN);
                auto type = parseType();
                expect(Token::KEY, Key::END);
                expect(Token::KEY, Key::LPAREN);
                auto body = parseExpr();
                expect(Token::KEY, Key::RPAREN);
                expr = MAKE<ast::CoerceExpr>(type, body);
            } else if (token.key() == Key::TRUE || token.key() == Key::FALSE) {
                expr = MAKE<ast::BoolExpr>(token.key() == Key::TRUE);
            }
        } else if (token.is(Token::IDENT)) {
            ahead = token;
            auto name = parseQualifiedType();
            expr = MAKE<ast::NameExpr>(name);
        } else if (token.is(Token::INT)) {
            expr = MAKE<ast::IntExpr>(token.number());
        } else if (token.is(Token::STRING)) {
            expr = MAKE<ast::StringExpr>(token.string());
        } else if (token.is(Token::CHAR)) {
            expr = MAKE<ast::CharExpr>(token.letter());
        }

        if (!expr) {
            ahead = token;
            return nullptr;
        }

        while (true) {
            if (eatKey(Key::LSQUARE)) {
                auto index = parseExpr();
                expect(Token::KEY, Key::RSQUARE);
                expr = MAKE<ast::SubscriptExpr>(expr, index);
            } else if (eatKey(Key::LPAREN)) {
                auto args = parseCallArgs();
                expr = MAKE<ast::CallExpr>(expr, args);
            } else if (eatKey(Key::DOT)) {
                expr = MAKE<ast::AccessExpr>(expr, MAKE<ast::Ident>(eat(Token::IDENT)), false);
            } else if (eatKey(Key::ARROW)) {
                expr = MAKE<ast::AccessExpr>(expr, MAKE<ast::Ident>(eat(Token::IDENT)), true);
            } else {
                break;
            }
        }

        return expr;
    }

    vec<ptr<ast::CallArg>> Parser::parseCallArgs() {
        bool name = false;
        vec<ptr<ast::CallArg>> args;

        while (true) {
            if (eatKey(Key::RPAREN)) {
                break;
            }

            if (eatKey(Key::DOT)) {
                name = true;
                auto key = MAKE<ast::Ident>(expect(Token::IDENT));
                expect(Token::KEY, Key::ASSIGN);
                auto body = parseExpr();
                args.push_back(MAKE<ast::CallArg>(key, body));
            } else {
                if (name) {
                    throw std::runtime_error("unnamed arg after named arg");
                }
                auto body = parseExpr();
                args.push_back(MAKE<ast::CallArg>(nullptr, body));
            }

            if (!eatKey(Key::RPAREN)) {
                expect(Token::KEY, Key::COMMA);
            }
        }

        return args;
    }

    //
    // helper functions
    //

    Token Parser::eat(Token::Type type, Key key) {
        Token token = next();

        if (!token.is(type)) {
            ahead = token;
            return {};
        }

        if (type == Token::KEY && key != Key::INVALID && token.key() != key) {
            ahead = token;
            return {};
        }

        return token;
    }

    Token Parser::expect(Token::Type type, Key key) {
        auto token = eat(type, key);

        if (!token.valid()) {
            throw std::runtime_error("unexpected token");
        }

        return token;
    }

    bool Parser::eatKey(Key key) {
        return eat(Token::KEY, key);
    }

    Token Parser::next() {
        auto temp = peek();
        ahead = {};
        return temp;
    }

    Token Parser::peek() {
        if (!ahead.valid()) {
            ahead = lexer->read();
        }

        return ahead;
    }
}
