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
        ptr<ast::Enum> parseEnum();
        ptr<ast::Variant> parseVariant();
        ptr<ast::Attributes> parseAttributes();

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

                    if (!eat(Token::KEY, sep)) {
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
