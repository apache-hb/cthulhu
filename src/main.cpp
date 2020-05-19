#include <variant>
#include <string>
#include <memory>
#include <optional>
#include <vector>

enum class Keyword {
    DEF, // def
    INCLUDE, // include
    TYPE, // type
    STRUCT, // struct
    UNION, // union
    ENUM,
    VARIANT,
    OBJECT,
    CONST,
    VAR,

    ADD, // +
    ADDEQ, // +=

    SUB, // -
    SUBEQ, // -=

    DIV, // /
    DIVEQ, // /=

    EQ, // ==
    NEQ, // !=

    ARROW, // ->
    BIGARROW, // =>
    ASSIGN, // :=
    COLON, // :
    COLON2, // ::

    SEMICOLON, // ;

    COMMA, // ,

    LPAREN, // (
    RPAREN, // )

    LBRACE, // {
    RBRACE, // }

    LSQUARE, // [
    RSQUARE // ]
};

const char* key_string(Keyword key) {
    switch(key) {
    case Keyword::DEF: return "def";
    case Keyword::INCLUDE: return "include";
    case Keyword::TYPE: return "type";
    case Keyword::STRUCT: return "struct";
    case Keyword::UNION: return "union";
    case Keyword::ENUM: return "enum";
    case Keyword::VARIANT: return "variant";
    case Keyword::OBJECT: return "object";
    case Keyword::CONST: return "const";
    case Keyword::VAR: return "var";

    case Keyword::ADD: return "+";
    case Keyword::ADDEQ: return "+=";

    case Keyword::SUB: return "-";
    case Keyword::SUBEQ: return "-=";

    case Keyword::DIV: return "/";
    case Keyword::DIVEQ: return "/=";

    case Keyword::EQ: return "==";
    case Keyword::NEQ: return "!=";

    case Keyword::ARROW: return "->";
    case Keyword::BIGARROW: return "=>";
    case Keyword::ASSIGN: return ":=";
    case Keyword::COLON: return ":";
    case Keyword::COLON2: return "::";

    case Keyword::SEMICOLON: return ";";

    case Keyword::COMMA: return ",";

    case Keyword::LPAREN: return "(";
    case Keyword::RPAREN: return ")";

    case Keyword::LBRACE: return "{";
    case Keyword::RBRACE: return "}";

    case Keyword::LSQUARE: return "[";
    case Keyword::RSQUARE: return "]";
    default: return "unknown";
    }
}

struct FilePos {
    uint64_t line;
    uint64_t col;
    uint64_t dist;
};

struct File {
    File(FILE* f)
        : handle(f)
        , name("unnamed")
    { 
        ahead = '\n';
    }

    File(const char* path)
        : handle(fopen(path, "rt"))
        , name(path)
    { 
        ahead = '\n';
    }

    char peek() { return ahead; }
    char next() {
        char c = ahead;
        ahead = fgetc(handle);

        if(c == '\n') {
            pos.col = 0;
            pos.line++;
        } else {
            pos.line = 0;
        }
        pos.dist++;

        return c;
    }

    bool consume(char c) {
        if(peek() == c) {
            next();
            return true;
        }

        return false;
    }

    FilePos here() { return pos; }

    FILE* handle;
    const char* name;
    char ahead;

    FilePos pos = { 0, 0, 0 };
};

struct Token {
    enum {
        IDENT,
        KEYWORD,
        STRING,
        INT,
        FLOAT,
        CHAR,
        END,
        ERR,
        NEWLINE,
        INVALID
    } type;

    std::variant<
        std::string, // ident | string | error
        Keyword, // keyword
        uint64_t, // int | char
        double // float
    > data;

    FilePos pos = {};

    static Token ident(std::string str) {
        return Token{IDENT, str};
    }

    static Token key(Keyword key) {
        return Token{KEYWORD, key};
    }

    static Token string(std::string str) {
        return Token{STRING, str};
    }

    static Token err(std::string msg) {
        return Token{ERR, msg};
    }

    static Token end() {
        return Token{END, UINT64_C(0)};
    }

    static Token invalid() {
        return Token{INVALID, UINT64_C(0)};
    }

    static Token line() {
        return Token{NEWLINE, UINT64_C(0)};
    }

    bool is(decltype(type) t) const { return type == t; }

    std::string to_string() const {
        switch(type) {
        case KEYWORD:
            return std::string("KEYWORD(") + key_string(std::get<Keyword>(data)) + ")";
        case IDENT:
            return std::get<std::string>(data);
        case STRING:
            return "STRING";
        case INT:
            return "INT";
        case FLOAT:
            return "FLOAT";
        case CHAR:
            return "CHAR";
        case END:
            return "EOF";
        case ERR:
            return std::get<std::string>(data);
        case NEWLINE:
            return "NEWLINE";
        default:
            return "OH NO";
        }
    }
};

struct Lexer {
    Lexer(File f)
        : file(f)
    { }

    Token next() {
        char c = file.next();
        Token tok;
        auto here = file.here();

        switch(c) {
        // muh performance
        case 'A': case 'B': case 'C': case 'D':
        case 'E': case 'F': case 'G': case 'H':
        case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P':
        case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z':
        case 'a': case 'b': case 'c': case 'd':
        case 'e': case 'f': case 'g': case 'h':
        case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p':
        case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z': case '_': {
            std::string buffer = {c};
            while(isalnum(file.peek()) || file.peek() == '_') {
                buffer += file.next();
            }

            if(buffer == "type") {
                tok = Token::key(Keyword::TYPE);
            } else if(buffer == "def") {
                tok = Token::key(Keyword::DEF);
            } else if(buffer == "union") {
                tok = Token::key(Keyword::UNION);
            } else if(buffer == "enum") {
                tok = Token::key(Keyword::ENUM);
            } else if(buffer == "variant") {
                tok = Token::key(Keyword::VARIANT);
            } else if(buffer == "object") {
                tok = Token::key(Keyword::OBJECT);
            } else if(buffer == "const") {
                tok = Token::key(Keyword::CONST);
            } else if(buffer == "var") {
                tok = Token::key(Keyword::VAR);
            } else if(buffer == "include") {
                tok = Token::key(Keyword::INCLUDE);
            } else {
                tok = Token::ident(buffer);
            }
            break;
        }
        case '/': 
            if(file.consume('/')) {
                while(file.next() != '\n');
                tok = Token::line();
            } else if(file.consume('*')) {
                int depth = 1;
                while(depth) {
                    if(file.consume('*') && file.next() == '/') {
                        depth--;
                    } else if(file.consume('/') && file.next() == '*') {
                        depth++;
                    }
                }
                tok = Token::line();
            } else if(file.consume('=')) {
                tok = Token::key(Keyword::DIVEQ);
            } else {
                tok = Token::key(Keyword::DIV);
            }
            break;
        case ' ': case '\t': return next();
        case '=':
            if(file.consume('=')) {
                tok = Token::key(Keyword::EQ);
            } else {
                tok = Token::err("= is not a valid keyword, use := for assignment");
            }
            break;
        case ':':
            if(file.consume('=')) {
                tok = Token::key(Keyword::ASSIGN);
            } else if(file.consume(':')) {
                tok = Token::key(Keyword::COLON2);
            } else {
                tok = Token::key(Keyword::COLON);
            }
            break;
        case '-':
            tok = Token::key(file.consume('>') ? Keyword::ARROW : Keyword::SUB);
            break;
        case ';': 
            tok = Token::key(Keyword::SEMICOLON); 
            break;
        case '\n': 
            tok = Token::line();
            break;
        case '\r': 
            tok = file.next() == '\n' ? Token::line() : Token::err("invalid line feed");
            break;
        case '\v': 
            tok = Token::err("get some help");
            break;
        case '\0': 
            tok = Token::end();
            break;
        }

        printf("tok %s\n", tok.to_string().c_str());
        tok.pos = here;
        return tok;
    }

    File file;
};


using Path = std::vector<std::string>;

struct Include {
    Path path;
    std::optional<std::string> alias;
};

struct AST {
    enum {
        INCLUDE,
        INVALID
    } type;

    std::variant<
        Include,
        uint64_t
    > data;

    static AST invalid() {
        return AST{INVALID, UINT64_C(0)};
    }
};

struct Parser {
    Parser(Lexer l)
        : lex(l)
    { 
        ahead = Token::invalid();
    }

    Path _path() {
        Path out;
        do { out.push_back(ident()); } while(consume(Keyword::COLON2));
        return out;
    }

    AST _include() {
        Include out;
        out.path = _path();

        if(consume(Keyword::ARROW)) {
            out.alias = ident();
        }

        line();

        return AST{AST::INCLUDE, out};
    }

    AST _typedef() {
        return AST::invalid();
    }

    AST _def() {
        return AST::invalid();
    }

    AST next() {
        if(consume(Keyword::INCLUDE)) {
            return _include();
        } else if(consume(Keyword::TYPE)) {
            return _typedef();
        } else if(consume(Keyword::DEF)) {
            return _def();
        } else {
            return AST::invalid();
        }
    }

    bool consume(Keyword key) {
        auto tok = get();
        if(tok.is(Token::KEYWORD) && std::get<Keyword>(tok.data) == key) {
            return true;
        } else {
            ahead = tok;
            return false;
        }
    }

    std::string ident() {
        auto tok = get();
        if(tok.is(Token::IDENT)) {
            return std::get<std::string>(tok.data);
        } else {
            printf("expected ident got %s\n", tok.to_string().c_str());
            return "invalid";
        }
    }

    // get the next token
    Token get_tok() {
        auto tok = ahead;
        if(tok.is(Token::INVALID)) {
            tok = lex.next();
        }
        return tok;
    }

    // get the next token, skips newlines
    Token get() {
        auto tok = get_tok();
        while(tok.is(Token::NEWLINE)) {
            tok = get_tok();
        }
        return tok;
    }

    // expect a linefeed or a ;
    void line() {
        auto tok = get_tok();
        if(tok.is(Token::NEWLINE))
            return;
        
        if(tok.is(Token::KEYWORD) && std::get<Keyword>(tok.data) == Keyword::SEMICOLON)
            return;

        printf("expected linefeed got %s instead\n", tok.to_string().c_str());
    }

    Lexer lex;
    Token ahead;
};

int main(int argc, const char** argv) {
    if(argc == 1) {
        printf("oh no\n");
        return 1;
    }

    auto lex = Lexer(File(argv[1]));
    auto parse = Parser(lex);

    auto test = parse.next();
    printf("include ");
    auto inc = std::get<Include>(test.data);
    for(size_t i = 0; i < inc.path.size(); i++) {
        if(i != 0) {
            printf("::");
        }
        printf("%s", inc.path[i].c_str());
    }

    if(inc.alias) {
        printf("-> %s\n", inc.alias.value().c_str());
    }
}