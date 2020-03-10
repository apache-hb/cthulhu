#include "ctu.h"

namespace ctu
{
    std::shared_ptr<Type> default_backing = std::make_shared<Builtin>(BuiltinType::ufast);

    std::shared_ptr<Type> Parser::type()
    {
        auto tok = lex->next();

        if(tok->type() == tok_type<Ident>())
        {
            return std::make_shared<Name>(static_cast<Ident*>(tok.get())->ident);
        }
        else if(tok->type() == tok_type<Key>())
        {
            switch(static_cast<Key*>(tok.get())->key)
            {
                // struct
            case Keyword::klbrace:
                return parse_struct();
                // tuple
            case Keyword::klparen:
                return parse_tuple();
                // variant
            case Keyword::kvariant:
                return parse_variant();
                // enum
            case Keyword::kenum:
                return parse_enum();
                // union
            case Keyword::kunion:
                return parse_union();
                // pointer
            case Keyword::kmul:
                return parse_pointer();
                // array
            case Keyword::klsquare:
                return parse_array();
                // invalid keyword
            default:
                break;
            }
        }
        else
        {
            // error
        }
    }
}