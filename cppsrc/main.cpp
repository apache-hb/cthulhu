#include <stdint.h>
#include <ctype.h>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <optional>

struct FilePos {
    uint64_t line;
    uint64_t col;
    uint64_t dist;
};

enum class TokenType {
    STRING,
    IDENT,
    INT,
    FLOAT,
    INVALID,
    END,
    KEYWORD,
    NONE
};

#define TOKEN(name) \
    static constexpr TokenType typeof = TokenType::name; \
    virtual TokenType type() const override { return TokenType::name; }

struct Token {
    virtual ~Token() {}

    FilePos pos;

    virtual TokenType type() const { return TokenType::NONE; }
};

struct String : Token {
    std::string str;

    TOKEN(STRING)
};

struct Ident : Token {
    std::string data;
    Ident(std::string&& d)
        : data(d)
    { }

    TOKEN(IDENT)
};

struct Int : Token {
    int64_t num;
    Int(int64_t n)
        : num(n)
    { }

    TOKEN(INT)
};

struct Float : Token {
    double val;

    TOKEN(FLOAT)
};

struct Invalid : Token {
    std::string reason;
    Invalid(std::string r)
        : reason(r)
    { }

    TOKEN(INVALID)
};

struct End : Token {
    TOKEN(END)
};

struct Keyword : Token {
    enum {
        TYPE,       // type
        IMPORT,     // import
        ENUM,       // enum
        UNION,      // union
        VARIANT,    // variant

        NOT,        // !
        BITNOT,     // ~

        EQ,         // ==
        NEQ,        // !=
    } key;

    using Key = decltype(key);

    Keyword(Key k)
        : key(k)
    { }

    TOKEN(KEYWORD)
};

struct Lexer {
    Lexer(FILE* input)
        : in(input)
    {
        ahead = fgetc(in);
    }

    template<typename T>
    std::unique_ptr<T> expect() {
        auto tok = next();
        if(tok->type() == T::typeof) {
            return std::unique_ptr<T>(std::move(tok));
        }
        
        return std::make_unique<T>();
    }
    
    std::unique_ptr<Token> next() {
        auto tok = parse();
        tok->pos = here;
        return tok;
    }

    std::unique_ptr<Token> parse() {
        auto c = skip_whitespace();

        while(c == '#')
            c = skip_comment();

        if(c == EOF)
            return std::make_unique<End>();

        here = pos;

        if(isalpha(c) || c == '_') {
            std::string buffer = {c};
            c = peek();
            while(isalnum(c) || c == '_')
                buffer += get();

            return std::unique_ptr<Token>(key_or_ident(std::move(buffer)));
        } else if(isdigit(c)) {
            if(c == '0') {
                return std::unique_ptr<Token>(digit());
            }

            return std::unique_ptr<Token>(number());
        } else {
            return std::unique_ptr<Token>(symbol(c));
        }
    }

private:
    Token* symbol(char c) {
        switch(c) {
        case '=': 
            if(consume('=')) 
                return new Keyword(Keyword::EQ);
            else 
                return new Invalid("= is not a valid keyword, variables are assigned with the := keyword");
        case '!':
            if(consume('='))
                return new Keyword(Keyword::NEQ);
            else
                return new Keyword(Keyword::NOT);
        default:
            return new Invalid("invalid keyword");
        }
    }

    Token* number() {
        return nullptr;
    }

    Token* hex() {
        int64_t val = 0;

        while(isxdigit(peek())) {
            auto c = get();

            c = (c & 0xF) + (c >> 6) | ((c >> 3) & 0x8);

            val = (val << 4) | (uint64_t)c;
        }

        return new Int(val);
    }

    Token* bin() {
        int64_t val = 0;

        while(peek() == '0' || peek() == '1') {
            auto c = get();
            val |= c - '0';
            val <<= 1;
        }

        return new Int(val);
    }

    Token* digit() {
        auto c = peek();
        if(c == 'x') {
            return hex();
        } else if(c == 'b') {
            return bin();
        }

        return new Invalid("numbers other than 0 cannot begin with 0");
    }

    Token* key_or_ident(std::string&& str) {
        if(str == "type")
            return new Keyword(Keyword::TYPE);
        else if(str == "import")
            return new Keyword(Keyword::IMPORT);
        else if(str == "enum")
            return new Keyword(Keyword::ENUM);
        else if(str == "union")
            return new Keyword(Keyword::UNION);
        else if(str == "variant")
            return new Keyword(Keyword::UNION);
        else
            return new Ident(std::move(str));
        
    }

    char skip_whitespace() {
        auto c = get();

        while(isspace(c))
            c = get();

        return c;
    }

    char skip_comment() {
        auto c = get();

        while(c != '\n')
            c = get();

        return skip_whitespace();
    }

    char get() {
        auto c = ahead;
        ahead = fgetc(in);

        pos.dist++;
        if(c == '\n') {
            pos.line++;
            pos.col = 0;
        } else {
            pos.col++;
        }

        return c;
    }

    char peek() {
        return ahead;
    }

    bool consume(char c) {
        if(peek() == c) {
            get();
            return true;
        }

        return false;
    }

    char ahead;

    FilePos here;
    FilePos pos = { 0, 0, 0 };
    FILE* in;
};

struct AST {
    FilePos pos;
};

struct Expr : AST { };
struct Stmt : AST { };

struct TypeAttribute : Stmt { };

struct Packed : TypeAttribute {
    Expr width;
};

struct Type : AST { 
    std::vector<TypeAttribute> attribs;
};

struct Import : Stmt {
    std::vector<std::string> path;
    std::optional<std::string> alias;
};

struct TypeDef : Stmt {
    std::string name;
    Type type;
};

struct Program {
    std::optional<std::string> name;
    std::vector<Import> imports;
    std::vector<Stmt> body;
};

struct Name : Type {
    std::string name;
};

struct Struct : Type {
    std::map<std::string, Type> fields;
};

struct Tuple : Type {
    std::vector<Type> fields;
};

struct Enum : Type {
    Type backing;
    std::map<std::string, Expr> values;
};

struct Union : Type {
    std::map<std::string, Type> fields;
};

struct Variant : Type {
    Type backing;
    std::map<std::string, std::pair<Expr, Type>> fields;
};


struct Unary : Expr {
    enum {
        POSITIVE,   // +expr
        NEGATIVE,   // -expr
        NOT,        // !expr
        FLIP,       // ~expr
        DEREF       // *expr
    } op;

    Expr expr;
};

struct StructInit : Expr {
    std::map<std::string, Expr> inits;
};

struct TupleInit : Expr {
    std::vector<Expr> inits;
};

struct ArrayInit : Expr {
    std::vector<Expr> inits;
};


struct Parser {
    Parser(Lexer l)
        : lex(l)
    { }

    Import parse_import() {
        std::vector<std::string> path = {};

        do { path.push_back() }
    }

private:
    Lexer lex;
};

struct FileParser : Parser {
    FileParser(Lexer lex)
        : Parser(lex)
    { }

    Program parse() {

    }
};

struct StreamParser : Parser {
    StreamParser(Lexer lex)
        : Parser(lex)
    { }

    Stmt next() {

    }
};

int main(int argc, const char** argv) {

}