#include "ctu.h"

namespace ctu
{
    Global Parser::global()
    {

    }

    TypeDef Parser::typedecl()
    {

    }

    Type Parser::type()
    {

    }

    std::map<std::string, Type> Parser::func_args()
    {
        
    }

    Func Parser::func()
    {
        Func ret;
        ret.name = next<Ident>().ident;

        expect(Keyword::klparen);

        ret.args = func_args();
    }

    AST Parser::parse()
    {
        AST ret;
        for(;;)
        {
            auto tok = lex->next();
            if(auto key = std::get_if<Key>(&tok))
            {
                switch(key->key)
                {
                case Keyword::klet:
                    ret.push_back(global());
                case Keyword::ktype:
                    ret.push_back(typedecl());
                case Keyword::kdef:
                    ret.push_back(func());
                default:
                    // TODO: invalid keyword
                    return ret;
                }
            }
            else if(std::holds_alternative<Eof>(tok))
            {
                // reached the end of the file
                return ret;
            }
            else
            {
                // TODO: error
                return ret;
            }
        }
    }
}