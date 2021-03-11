#pragma once

#include "fwd.hpp"
#include "ast.hpp"
#include "token.hpp"

namespace cthulhu {
    struct Parser {
        Parser(Lexer* lexer);

        ptr<ast::Unit> parseUnit();
        ptr<ast::Node> parseImport();

        ptr<ast::Decl> parseDecl();
        ptr<ast::Alias> parseAlias();
        ptr<ast::Record> parseRecord();
        ptr<ast::Union> parseUnion();
        ptr<ast::Variant> parseVariant();
        ptr<ast::Var> parseVariable(bool semi = true);
        ptr<ast::Decorated> parseDecorated();

        ptr<ast::Stmt> parseStmt();
        ptr<ast::Return> parseReturn();
        ptr<ast::Compound> parseCompound();

        ptr<ast::Type> parseType();
        ptr<ast::PointerType> parsePointerType();
        ptr<ast::MutableType> parseMutableType();
        ptr<ast::ArrayType> parseArrayType();
        ptr<ast::ClosureType> parseClosureType();
        ptr<ast::NameType> parseNameType();
        ptr<ast::QualifiedType> parseQualifiedType();

        ptr<ast::Expr> parseExpr();
        ptr<ast::Expr> parseBinaryExpr(int precedence);
        ptr<ast::Expr> parsePrimaryExpr();

        ptr<ast::Attribute> parseAttribute();
        ptr<ast::Field> parseField(bool semi);
        ptr<ast::Case> parseCase();
        vec<ptr<ast::VarName>> parseVarNames();
        ptr<ast::VarName> parseVarName();
        vec<ptr<ast::CallArg>> parseCallArgs(bool empty = true);
        ptr<ast::Ident> parseIdent();

    private:
        Token next();
        Token peek();

        Token eat(Token::Type type, Key key = Key::INVALID);
        Token expect(Token::Type type, Key key = Key::INVALID);
        bool eatKey(Key key);

        template<typename T, typename F>
        vec<ptr<T>> collect(Key sep, F&& func, bool once = true) {
            vec<ptr<T>> out;

            if (once) {
                do { 
                    out.push_back(func()); 
                } while (eat(Token::KEY, sep));
            } else {
                while (true) {
                    if (auto it = func(); it) {
                        out.push_back(it);
                    } else {
                        break;
                    }

                    if (sep != Key::INVALID && !eat(Token::KEY, sep)) {
                        break;
                    }
                }
            }

            return out;
        }

        Lexer* lexer;
        Token ahead;
    };
}
