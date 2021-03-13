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

    ast::UnaryExpr::UnaryOp unop(Token token) {
        if (!token.is(Token::KEY)) {
            return ast::UnaryExpr::INVALID;
        }

        switch (token.key()) {
        case Key::ADD: return ast::UnaryExpr::POS;
        case Key::SUB: return ast::UnaryExpr::NEG;
        case Key::BITAND: return ast::UnaryExpr::REF;
        case Key::MUL: return ast::UnaryExpr::DEREF;
        case Key::FLIP: return ast::UnaryExpr::FLIP;
        case Key::NOT: return ast::UnaryExpr::NOT;
        default: return ast::UnaryExpr::INVALID;
        }
    }

    ast::BinaryExpr::BinaryOp binop(Token token) {
        if (!token.is(Token::KEY)) {
            return ast::BinaryExpr::INVALID;
        }

        switch (token.key()) {
        case Key::ADD: return ast::BinaryExpr::ADD;
        case Key::SUB: return ast::BinaryExpr::SUB;
        case Key::MUL: return ast::BinaryExpr::MUL;
        case Key::MOD: return ast::BinaryExpr::MOD;
        case Key::DIV: return ast::BinaryExpr::DIV;
        case Key::BITAND: return ast::BinaryExpr::BITAND;
        case Key::BITOR: return ast::BinaryExpr::BITOR;
        case Key::XOR: return ast::BinaryExpr::XOR;
        case Key::AND: return ast::BinaryExpr::AND;
        case Key::OR: return ast::BinaryExpr::OR;
        case Key::SHL: return ast::BinaryExpr::SHL;
        case Key::SHR: return ast::BinaryExpr::SHR;
        case Key::LT: return ast::BinaryExpr::LT;
        case Key::LTE: return ast::BinaryExpr::LTE;
        case Key::GT: return ast::BinaryExpr::GT;
        case Key::GTE: return ast::BinaryExpr::GTE;
        case Key::EQ: return ast::BinaryExpr::EQ;
        case Key::NEQ: return ast::BinaryExpr::NEQ;
        default: return ast::BinaryExpr::INVALID;
        }
    }

    ast::Assign::Op assop(Token token) {
        if (!token.is(Token::KEY)) {
            return ast::Assign::INVALID;
        }

        switch (token.key()) {
        case Key::ASSIGN: return ast::Assign::ASSIGN;
        case Key::ADDEQ: return ast::Assign::ADDEQ;
        case Key::SUBEQ: return ast::Assign::SUBEQ;
        case Key::DIVEQ: return ast::Assign::DIVEQ;
        case Key::MODEQ: return ast::Assign::MODEQ;
        case Key::MULEQ: return ast::Assign::MULEQ;
        case Key::SHLEQ: return ast::Assign::SHLEQ;
        case Key::SHREQ: return ast::Assign::SHREQ;
        case Key::BITOREQ: return ast::Assign::OREQ;
        case Key::BITANDEQ: return ast::Assign::ANDEQ;
        case Key::XOREQ: return ast::Assign::XOREQ;
        default: return ast::Assign::INVALID;
        }
    }
}

namespace cthulhu {
    Parser::Parser(Lexer* lexer)
        : lexer(lexer)
    { }

    //
    // toplevel decls
    //

    ptr<ast::Decl> Parser::parseDecl() {
        TRY(parseAlias());
        TRY(parseRecord());
        TRY(parseUnion());
        TRY(parseVariant());
        TRY(parseVariable());
        TRY(parseFunction());
        TRY(parseDecorated());
        
        return nullptr;
    }

    ptr<ast::Alias> Parser::parseAlias() {
        if (!eatKey(Key::USING)) {
            return nullptr;
        }

        auto name = parseIdent();

        expect(Token::KEY, Key::ASSIGN);

        auto type = parseType();

        expect(Token::KEY, Key::SEMI);

        return MAKE<ast::Alias>(name, type);
    }

    ptr<ast::Record> Parser::parseRecord() {
        if (!eatKey(Key::RECORD)) {
            return nullptr;
        }

        auto name = parseIdent();

        expect(Token::KEY, Key::LBRACE);
        auto fields = collect<ast::Field>(Key::INVALID, [&] { return parseField(true); }, false);
        expect(Token::KEY, Key::RBRACE);

        return MAKE<ast::Record>(name, fields);
    }

    ptr<ast::Union> Parser::parseUnion() {
        if (!eatKey(Key::UNION)) {
            return nullptr;
        }

        auto name = parseIdent();

        expect(Token::KEY, Key::LBRACE);
        auto fields = collect<ast::Field>(Key::INVALID, [&] { return parseField(true); }, false);
        expect(Token::KEY, Key::RBRACE);

        return MAKE<ast::Union>(name, fields);
    }

    ptr<ast::Variant> Parser::parseVariant() {
        if (!eatKey(Key::VARIANT)) {
            return nullptr;
        }

        auto name = parseIdent();

        auto parent = eatKey(Key::COLON) ? parseQualifiedType() : nullptr;

        expect(Token::KEY, Key::LBRACE);
        auto cases = collect<ast::Case>(Key::INVALID, [&] { return parseCase(); }, false);
        expect(Token::KEY, Key::RBRACE);

        return MAKE<ast::Variant>(name, parent, cases);
    }

    ptr<ast::Var> Parser::parseVariable(bool semi) {
        bool mut;
        if (eatKey(Key::LET)) {
            mut = false;
        } else if (eatKey(Key::VAR)) {
            mut = true;
        } else {
            return nullptr;
        }

        auto names = parseVarNames();
        auto init = eatKey(Key::ASSIGN) ? parseExpr() : nullptr;

        if (semi) {
            expect(Token::KEY, Key::SEMI);
        }

        return MAKE<ast::Var>(names, init, mut);
    }

    ptr<ast::Function> Parser::parseFunction() {
        if (!eatKey(Key::DEF)) {
            return nullptr;
        }

        auto name = parseIdent();
        vec<ptr<ast::Param>> params;
        if (eatKey(Key::LPAREN)) {
            params = parseFunctionParams();
        }

        auto result = eatKey(Key::COLON) ? parseType() : nullptr;

        ptr<ast::Stmt> body;

        if (eatKey(Key::SEMI)) {
            body = nullptr;
        } else if (eatKey(Key::ASSIGN)) {
            body = parseExpr();
            expect(Token::KEY, Key::SEMI);
        } else {
            body = parseCompound();
        }

        return MAKE<ast::Function>(name, params, result, body);
    }

    vec<ptr<ast::Param>> Parser::parseFunctionParams() {
        bool init = false;

        vec<ptr<ast::Param>> params;
        while (true) {
            if (eatKey(Key::RPAREN)) {
                break;
            }

            auto name = parseIdent();
            expect(Token::KEY, Key::COLON);
            auto type = parseType();

            ptr<ast::Expr> val = nullptr;

            if (eatKey(Key::ASSIGN)) {
                init = true;
                val = parseExpr();
            } else {
                if (init) {
                    throw std::runtime_error("uninitialized parameter after default initialized parameter");
                }
            }

            params.push_back(MAKE<ast::Param>(name, type, val));

            if (eatKey(Key::RPAREN)) {
                break;
            } else {
                expect(Token::KEY, Key::COMMA);
            }
        }

        return params;
    }

    vec<ptr<ast::VarName>> Parser::parseVarNames() {
        if (eatKey(Key::LSQUARE)) {
            auto out = collect<ast::VarName>(Key::COMMA, [&] { return parseVarName(); }, true);
            expect(Token::KEY, Key::RSQUARE);
            return out;
        } else {
            return vec<ptr<ast::VarName>>({ parseVarName() });
        }
    }

    ptr<ast::VarName> Parser::parseVarName() {
        auto name = parseIdent();
        auto type = eatKey(Key::COLON) ? parseType() : nullptr;
        return MAKE<ast::VarName>(name, type);
    }

    ptr<ast::Field> Parser::parseField(bool semi) {
        if (auto name = parseIdent(); name) {
            expect(Token::KEY, Key::COLON);
            auto type = parseType();
            
            if (semi) {
                expect(Token::KEY, Key::SEMI);
            }

            return MAKE<ast::Field>(name, type);
        } else {
            return nullptr;
        }
    }

    ptr<ast::Case> Parser::parseCase() {
        if (!eatKey(Key::CASE)) {
            return nullptr;
        }

        auto name = parseIdent();
        vec<ptr<ast::Field>> fields;
        if (eatKey(Key::LPAREN)) {
            fields = collect<ast::Field>(Key::COMMA, [&] { return parseField(false); }, true);
            expect(Token::KEY, Key::RPAREN);
        }

        auto init = eatKey(Key::ASSIGN) ? parseExpr() : nullptr;

        expect(Token::KEY, Key::SEMI);

        return MAKE<ast::Case>(name, init, fields);
    }

    ptr<ast::Decorated> Parser::parseDecorated() {
        if (!eatKey(Key::AT)) {
            return nullptr;
        }

        vec<ptr<ast::Attribute>> attribs;
        do {
            if (eatKey(Key::LSQUARE)) {
                do {
                    attribs.push_back(parseAttribute());
                } while (eatKey(Key::COMMA));
                expect(Token::KEY, Key::RSQUARE);
            } else {
                attribs.push_back(parseAttribute());
            }
        } while (eatKey(Key::AT));

        auto decl = parseDecl();

        return MAKE<ast::Decorated>(attribs, decl);
    }

    ptr<ast::Attribute> Parser::parseAttribute() {
        auto name = parseQualifiedType();

        vec<ptr<ast::CallArg>> args;
        if (eatKey(Key::LPAREN)) {
            args = parseCallArgs();
        }

        return MAKE<ast::Attribute>(name, args);
    }

    ptr<ast::Node> Parser::parseImport() {
        if (!eatKey(Key::USING)) {
            return nullptr;
        }

        ptr<ast::Node> node;

        auto path = collect<ast::Ident>(Key::COLON2, [&] { return parseIdent(); });
        
        if (eatKey(Key::ASSIGN)) {
            auto type = parseType();
            node = MAKE<ast::Alias>(path[0], type);
        } else {
            if (eatKey(Key::LPAREN)) {
                if (eatKey(Key::DOT3)) {
                    node = MAKE<ast::Import>(path, false);
                } else {
                    auto items = collect<ast::Ident>(Key::COMMA, [&] { return parseIdent(); });
                    node = MAKE<ast::Import>(path, false, items);
                }

                expect(Token::KEY, Key::RPAREN);
            } else {
                node = MAKE<ast::Import>(path, true);
            }
        }

        expect(Token::KEY, Key::SEMI);

        return node;
    }

    ptr<ast::Unit> Parser::parseUnit() {
        vec<ptr<ast::Import>> imports;
        vec<ptr<ast::Decl>> decls;

        while (true) {
            if (auto node = parseImport(); node) {
                if (auto alias = SELF<ast::Alias>(node); alias) {
                    decls.push_back(alias);
                    break;
                } else {
                    imports.push_back(SELF<ast::Import>(node));
                }
            } else {
                break;
            }
        }

        while (true) {
            if (auto decl = parseDecl(); decl) {
                decls.push_back(decl);
            } else {
                break;
            }
        }

        return MAKE<ast::Unit>(imports, decls);
    }

    //
    // stmt parsing
    // 

    ptr<ast::Stmt> Parser::parseStmt() {
        TRY(parseCompound());
        TRY(parseReturn());
        TRY(parseVariable(true));
        TRY(parseFunction());
        TRY(parseWhile());
        TRY(parseBranch());

        auto expr = parseExpr();

        if (!expr) {
            return nullptr;
        }

        if (auto op = assop(peek()); op != ast::Assign::INVALID) {
            next();
            auto val = parseExpr();
            expect(Token::KEY, Key::SEMI);

            return MAKE<ast::Assign>(op, expr, val);
        } else {
            expect(Token::KEY, Key::SEMI);
            return expr;
        }
    }

    ptr<ast::Branch> Parser::parseBranch() {
        if (!eatKey(Key::IF)) {
            return nullptr;
        }

        vec<ptr<ast::If>> branches;

        auto cond = parseExpr();
        auto body = parseCompound();

        branches.push_back(MAKE<ast::If>(cond, body));

        while (true) {
            if (eatKey(Key::ELSE)) {
                if (eatKey(Key::IF)) {
                    cond = parseExpr();
                    body = parseCompound();
                    branches.push_back(MAKE<ast::If>(cond, body));
                } else {
                    body = parseCompound();
                    branches.push_back(MAKE<ast::If>(nullptr, body));
                    break;
                }
            } else {
                break;
            }
        }

        return MAKE<ast::Branch>(branches);
    }

    ptr<ast::Return> Parser::parseReturn() {
        if (!eatKey(Key::RETURN)) {
            return nullptr;
        }

        if (eatKey(Key::SEMI)) {
            return MAKE<ast::Return>(nullptr);
        } else {
            auto expr = parseExpr();
            expect(Token::KEY, Key::SEMI);
            return MAKE<ast::Return>(expr);
        }
    }

    ptr<ast::While> Parser::parseWhile() {
        if (!eatKey(Key::WHILE)) {
            return nullptr;
        }

        auto cond = parseExpr();

        auto body = parseCompound();

        ptr<ast::Stmt> other;
        if (eatKey(Key::ELSE)) {
            other = parseCompound();
        }

        return MAKE<ast::While>(cond, body, other);
    }

    ptr<ast::Compound> Parser::parseCompound() {
        if (!eatKey(Key::LBRACE)) {
            return nullptr;
        }

        vec<ptr<ast::Stmt>> stmts;

        while (true) {
            if (auto stmt = parseStmt(); stmt) {
                stmts.push_back(stmt);
            } else {
                break;
            }
        }

        expect(Token::KEY, Key::RBRACE);

        return MAKE<ast::Compound>(stmts);
    }

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
        if (auto name = parseIdent(); name) {
            return MAKE<ast::NameType>(name);
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
                lhs = MAKE<ast::BinaryExpr>(binop(token), lhs, rhs);
            }
        }

        return lhs;
    }

    ptr<ast::Expr> Parser::parsePrimaryExpr() {
        Token token = next();

        ptr<ast::Expr> expr;

        if (token.is(Token::KEY)) {
            if (auto op = unop(token); op != ast::UnaryExpr::INVALID) {
                expr = MAKE<ast::UnaryExpr>(op, parsePrimaryExpr());
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
                expr = MAKE<ast::AccessExpr>(expr, parseIdent(), false);
            } else if (eatKey(Key::ARROW)) {
                expr = MAKE<ast::AccessExpr>(expr, parseIdent(), true);
            } else if (eatKey(Key::QUESTION)) {
                auto yes = parseExpr();
                expect(Token::KEY, Key::COLON);
                auto no = parseExpr();
                expr = MAKE<ast::TernaryExpr>(expr, yes, no);
            } else {
                break;
            }
        }

        return expr;
    }

    vec<ptr<ast::CallArg>> Parser::parseCallArgs(bool empty) {
        bool name = false;
        vec<ptr<ast::CallArg>> args;

        if (empty && eatKey(Key::RPAREN)) {
            return args;
        }

        while (true) {
            if (eatKey(Key::DOT)) {
                name = true;
                auto key = parseIdent();
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

            if (eatKey(Key::RPAREN)) {
                break;
            } else {
                expect(Token::KEY, Key::COMMA);
            }
        }

        return args;
    }

    ptr<ast::Ident> Parser::parseIdent() {
        if (Token id = eat(Token::IDENT); id.valid()) {
            return MAKE<ast::Ident>(id);
        } else {
            return nullptr;
        }
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
            if (type == Token::KEY) {
#define OP(id, str) case Key::id: fprintf(stderr, "expected keyword: %s\n", str); break;
#define KEY(id, str) case Key::id: fprintf(stderr, "expected keyword: %s\n", str); break;
                switch (key) {
#include "keys.inc"
                default: fprintf(stderr, "keyword: invalid\n"); break;
                }
            }

            fprintf(stderr, "got");
            if (ahead.is(Token::KEY)) {
                #define OP(id, str) case Key::id: fprintf(stderr, "keyword: %s\n", str); break;
#define KEY(id, str) case Key::id: fprintf(stderr, "keyword: %s\n", str); break;
                switch (ahead.key()) {
#include "keys.inc"
                default: fprintf(stderr, "keyword: invalid\n"); break;
                }
                fprintf(stderr, "instead\n");
            }
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
