#include "lexer.cpp"

#include <vector>
#include <string>

namespace ct {
    using str = std::string;

    template<typename T>
    using vec = std::vector<T>;

    template<typename... T>
    using var = std::variant<T...>;

    struct Node {};

    struct Type : Node {};
    struct Builtin : Type {
        enum { 
            U8, U16, U32, U64,
            I8, I16, I32, I64
        } type;
    };

    struct Decl : Node {};

    struct Alias : Decl {
        Alias(str n, Type* t)
            : name(n)
            , type(t)
        { }
        str name;
        Type* type;
    };

    struct Unit : Node {
        vec<Decl*> decls;
    };

    struct Parser {
        Parser(Lexer l)
            : ahead(Token::invalid)
            , lex(l)
        { }

        Type* parse_type() {
            
        }

        Alias* parse_alias() {
            auto name = expect<Token::ident>();
            expect<Token::key>(Keyword::ASSIGN);
            auto type = parse_type();
            expect<Token::key>(Keyword::SEMICOLON);
            return new Alias(name, type);
        }

        Decl* parse_decl() {
            if (consume<Token::key>(Keyword::ALIAS)) {
                return parse_alias();
            }

            return nullptr;
        }

        Unit parse() {

        }

        template<Token::type_t T, typename V>
        bool consume(const V& val) {
            auto tok = next();
            if (tok.is(T, val)) {
                return true;
            }
            ahead = tok;
            return false;
        }

        template<Token::type_t T>
        Token::underlying<T>::type expect() {
            auto tok = next();
            if (tok.is(T)) {
                return std::get<typename Token::underlying<T>::type>(tok.data);
            }
            printf("bbbb\n");
            std::abort();
        }

        template<Token::type_t T, typename V>
        void expect(const V& val) {
            if (expect<T>() != val) {
                printf("Aaaaaa\n");
                std::abort();
            }
        }

        Token next() {
            auto tok = ahead;
            if (tok.type == Token::invalid) {
                tok = lex.next();
            }
            
            ahead = Token(Token::invalid);
            
            return tok;
        }

        Token ahead;
        Lexer lex;
    };
}