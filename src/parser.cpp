#include "lexer.cpp"

#include "ast.h"

namespace ct {

    template<typename L>
    struct Parser {

        template<typename... T>
        Parser(T&&... args)
            : lex(args...)
        { }

    private:
        std::vector<std::string> parse_name(Keyword key) {
            std::vector<std::string> path;
            do path.push_back(ident()); while (consume(key));
            return path;
        }

        std::vector<std::string> parse_import_items() {
            return consume(Keyword::MUL) ? std::vector<std::string>() : parse_name(Keyword::COMMA);
        }

        ast::Import parse_import() {
            auto path = parse_name(Keyword::COLON2);
            expect(Keyword::LPAREN);
            auto items = parse_import_items();
            expect(Keyword::RPAREN);
            return { path, items };
        }

        using TypeName = std::vector<std::string>;

        TypeName parse_typename() {
            // TODO: template arguments will be a thing eventually
            return parse_name(Keyword::COLON2);
        }

#define BUILTIN(name) static constexpr ast::Builtin name = ast::Builtin(ast::Builtin::name)
            BUILTIN(U8);
            BUILTIN(U16);
            BUILTIN(U32);
            BUILTIN(U64);
            BUILTIN(I8);
            BUILTIN(I16);
            BUILTIN(I32);
            BUILTIN(I64);
            BUILTIN(F32);
            BUILTIN(F64);
            BUILTIN(VOID);
            BUILTIN(BOOL);
            BUILTIN(CHAR);
            BUILTIN(UCHAR);
            BUILTIN(SHORT);
            BUILTIN(USHORT);
            BUILTIN(INT);
            BUILTIN(UINT);
            BUILTIN(LONG);
            BUILTIN(ULONG);
            BUILTIN(FLOAT);
            BUILTIN(DOUBLE);
            BUILTIN(ISIZE);
            BUILTIN(USIZE);
#undef BUILTIN

        using self_t = Parser<L>;

        ast::Type* builtin_type(TypeName path) {
            switch (crc32(path[0])) {
#define BUILTIN(name, type) case crc32(name): return &self_t::type;
            BUILTIN("u8", U8);
            BUILTIN("u16", U16);
            BUILTIN("u32", U32);
            BUILTIN("u64", U64);
            BUILTIN("i8", I8);
            BUILTIN("i16", I16);
            BUILTIN("i32", I32);
            BUILTIN("i64", I64);
            BUILTIN("f32", F32);
            BUILTIN("f64", F64);
            BUILTIN("void", VOID);
            BUILTIN("bool", BOOL);
            BUILTIN("char", CHAR);
            BUILTIN("uchar", UCHAR);
            BUILTIN("short", SHORT);
            BUILTIN("ushort", USHORT);
            BUILTIN("int", INT);
            BUILTIN("uint", UINT);
            BUILTIN("long", LONG);
            BUILTIN("ulong", ULONG);
            BUILTIN("float", FLOAT);
            BUILTIN("double", DOUBLE);
            BUILTIN("usize", USIZE);
            BUILTIN("isize", ISIZE);
#undef BUILTIN
            default:
                return new ast::Name(path);
            }
        }

        ast::Type* type_of(TypeName path) {
            if (path.size() == 1) {
                return builtin_type(path);
            } else {
                return new ast::Name(path);
            }
        }

        ast::Type* parse_type() {
            if (peek().is(Token::ident)) {
                auto name = parse_typename();
                return type_of(name);
            }
        }


        ast::Struct* parse_struct_body() {
            return nullptr;
        }

        std::tuple<std::string, ast::Struct*> parse_struct() {
            auto name = ident();
            expect(Keyword::LBRACE);
            auto body = parse_struct_body();
            expect(Keyword::RBRACE);

            return std::make_tuple(name, body);
        }

        ast::Body parse_body() {
            ast::Body body;

            while (true) {
                if (consume(Keyword::STRUCT)) {
                    auto [name, type] = parse_struct();
                    body.types[name] = type;
                } else {
                    break;
                }
            }

            return body;
        }

        std::vector<ast::Import> parse_imports() {
            std::vector<ast::Import> imports;
            while (true) {
                if (consume(Keyword::IMPORT)) {
                    imports.push_back(parse_import());
                    expect(Keyword::SEMICOLON);
                } else if (consume(Keyword::AT)) {
                    // TODO
                    printf("TODO: special\n");
                } else {
                    break;
                }
            }
            return imports;
        }

        void expect(Keyword key) {
            if (!consume(key)) {
                printf("expected %s\n", ct::str(key));
                std::exit(400);
            }
        }

        bool consume(Keyword key) {
            auto tok = next();
            if (tok.is(Token::key, key)) {
                return true;
            }

            ahead = tok;
            return false;
        }

        std::string ident() {
            auto tok = next();
            if (!tok.is(Token::ident)) {
                printf("expected ident\n");
                std::exit(400);
            }
            return std::get<std::string>(tok.data);
        }

        Token next() {
            auto tok = ahead;
            if (ahead.is(Token::invalid)) {
                tok = lex.next();
            }
            ahead = Token(Token::invalid);
            return tok;
        }

        const Token& peek() {
            ahead = next();
            return ahead;
        }

        Token ahead = Token(Token::invalid);
        L lex;

    public:
        ast::Program parse_program() {
            auto imports = parse_imports();
            auto body = parse_body();
            return { imports, body };
        }
    };
}