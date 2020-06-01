#pragma once

#include "lexer.h"
#include "ast.h"

namespace ct
{
    struct Parser
    {
        Parser(Lexer l)
            : lex(l)
            , ahead(Token::invalid())
        { }

        template<typename T, typename F>
        T _collect(F&& func)
        {
            T out;
            do { out.push_back(ident()); printf("%s\n", out.back().c_str()); } while (func());
            return out;
        }

        ast::Import _import()
        {
            auto path = _collect<ast::Path>([this] { return consume(Keyword::COLON2); });
            expect(Keyword::LPAREN);
            ast::Path deps = consume(Keyword::ELLIPSIS)
                ? ast::Path{}
                : _collect<ast::Path>([this] { return consume(Keyword::COMMA); });
            expect(Keyword::RPAREN);
            return { path, deps };
        }

        ast::Program parse()
        {
            std::vector<ast::Import> imports = {};
            while (consume(Keyword::IMPORT))
            {
                imports.push_back(_import());
                expect(Keyword::SEMICOLON);
            }

            return { imports };
        }

        bool consume(Keyword key)
        {
            Token tok = next();
            printf("here %d\n", (int)key);
            if (tok.type == Token::KEYWORD && std::get<Keyword>(tok.data) == key)
            {
                return true;
            }
            ahead = tok;
            return false;
        }

        void expect(Keyword key)
        {
            Token tok = next();
            printf("aaa %s\n", to_string(key));
            if (tok.type != Token::KEYWORD || std::get<Keyword>(tok.data) != key)
            {
                printf("oh no\n");
                std::exit(-2);
            }
        }

        std::string ident()
        {
            Token tok = next();
            if (tok.type != Token::IDENT)
            {
                printf("uhhh %d %s\n", tok.type, to_string(std::get<Keyword>(tok.data)));
            }
            return std::get<std::string>(tok.data);
        }

        Token next()
        {
            Token tok = ahead;
            if (tok.type == Token::INVALID)
                tok = lex.next();

            ahead = Token::invalid();
            return tok;
        }

        Lexer lex;
        Token ahead;
    };
}