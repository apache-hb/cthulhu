#pragma once

#include "ast.h"
#include "lex.h"

namespace ctu
{
    struct Parser
    {
        Parser(Lexer* l)
            : lex(l)
            , lookahead(lex->next())
        {}

        ast::Ident* parse_ident()
        {
            return new ast::Ident(expect<Ident>().name);
        }

        ast::Name* parse_dotted_name()
        {
            ast::Name* out = new ast::Name();

            out->path.push_back(parse_ident());
            while(consume<Key>(Keyword::_colon2))
            {
                out->path.push_back(parse_ident());
            }

            return out;
        }

        ast::Import* parse_import()
        {
            ast::Import* out = new ast::Import();
            
            out->path = parse_dotted_name();
            if(consume<Key>(Keyword::_arrow))
            {
                out->alias = parse_ident();
            }

            return out;
        }

        ast::Program* parse()
        {
            std::vector<ast::Import*> imports;

            auto key = expect<Key>();
            while(key.key == Keyword::_import)
            {
                imports.push_back(parse_import());
                key = expect<Key>(Keyword::_invalid);
            }

            for(;;)
            {
                if(key == Keyword::_type)
                {

                }
                else if(key == Keyword::_def)
                {

                }
            }

            return new ast::Program(imports);
        }

    private:
        template<typename T, typename D>
        bool consume(D v, LexFlags flags = FNone)
        {
            if(auto* val = std::get_if<T>(&lookahead.data); val != nullptr)
            {
                if(val->equals(v))
                {
                    return true;
                }
            }

            return false;
        }

        template<typename T>
        T expect(LexFlags flags = FNone)
        {
            auto tok = lex->next(flags);
            if(std::get_if<T>(&tok.data) == nullptr)
            {
                printf("unexpected token %s\n", ctu::to_string(tok).c_str());
                std::exit(400);
            }

            return std::get<T>(tok.data);
        }

        template<typename T>
        T expect(T fallback, LexFlags flags = FNone)
        {
            auto tok = lex->next(flags);
            if(std::get_if<T>(&tok.data) == nullptr)
            {
                return fallback;
            }


            return std::get<T>(tok.data);
        }

        Lexer* lex;
    };
}