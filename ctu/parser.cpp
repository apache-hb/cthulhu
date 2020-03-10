#include "ctu.h"

namespace ctu
{
    std::shared_ptr<Type> default_backing = std::make_shared<Builtin>(BuiltinType::ufast);

    std::shared_ptr<Struct> Parser::parse_struct()
    {
        std::map<std::string, std::shared_ptr<Type>> fields;

        for(;;)
        {
            auto tok = lex->next();
            if(tok_type<Ident>() == tok->type())
            {
                auto name = static_cast<Ident*>(tok.get())->ident;

                expect(Keyword::kcolon);

                fields[name] = type();

                auto k = expect<Key>();
                if(k->key == Keyword::kcomma)
                    continue;
                else if(k->key == Keyword::krbrace)
                    break;
                else {}
                    // error
            }
        }

        return std::make_shared<Struct>(fields);
    }

    std::shared_ptr<Tuple> Parser::parse_tuple()
    {
        std::vector<std::shared_ptr<Type>> vec;

        for(;;)
        {
            auto tok = lex->peek();
            if(tok_type<Key>() == tok->type())
            {
                if(std::static_pointer_cast<Key>(tok)->key == Keyword::klparen)
                    break;
                else
                    vec.push_back(type());

                
            }
        }
    }

    std::shared_ptr<Union> Parser::parse_union()
    {

    }

    std::shared_ptr<Enum> Parser::parse_enum()
    {

    }

    std::shared_ptr<Variant> Parser::parse_variant()
    {
        
    }

    std::shared_ptr<Ptr> Parser::parse_ptr()
    {
        return std::make_shared<Ptr>(type());
    }

    std::shared_ptr<Array> Parser::parse_array()
    {
        
    }

    std::shared_ptr<Name> Parser::parse_name()
    {
        return std::make_shared<Name>(expect<Ident>()->ident);
    }


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
                return parse_ptr();
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