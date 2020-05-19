#include <variant>
#include <string>
#include <memory>

enum class Keyword {
    DEF, // def
    INCLUDE, // include
    TYPE, // type
    STRUCT, // struct
    UNION, // union

    ASSIGN, // :=
    COLON, // :
    COLON2, // ::

    NEWLINE, // \n
    SEMICOLON, // ;

    COMMA, // ,

    LPAREN, // (
    RPAREN, // )

    LBRACE, // {
    RBRACE, // }

    LSQUARE, // [
    RSQUARE // ]
};

struct File {
    FILE* handle;
    const char* name;
};

struct FilePos {
    uint64_t line;
    uint64_t col;
    uint64_t dist;
    std::shared_ptr<File> file;
};

struct Token {
    enum {
        IDENT,
        KEYWORD,
        STRING,
        END,
    } type;

    std::variant<
        std::string, // ident | string
        Keyword // keyword
    > data;

    FilePos pos;

    static Token ident(std::string&& str) {
        return Token{IDENT, str};
    }

    static Token keyword(Keyword key) {
        return Token{KEYWORD, key};
    }

    static Token string(std::string&& str) {
        return Token{STRING, str};
    }

    static Token end() {
        return Token{END};
    }

    Token at(FilePos p) {
        pos = p;
        return *this;
    }
};

struct Lexer {
    Token next() {
        return Token::end().at({ 0, 0, 0, nullptr });
    }
};

int main(int argc, const char** argv) {
    
}