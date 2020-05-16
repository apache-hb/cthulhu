#include <string>
#include <vector>
#include <optional>
#include <variant>

using Path = std::vector<std::string>;

struct Include {
    Path path;
    std::optional<std::string> alias;
};

struct Program {
    std::vector<Include> includes;
};

struct SourcePos {
    struct Lexer* source;
    uint64_t dist;
    uint64_t line;
    uint64_t col;
};

typedef enum {
    TYPE,
    DEF,
    INCLUDE,

    SEMICOLON,
    NEWLINE
} Keyword;

struct Token {
    enum {
        KEY,
        IDENT,
        END
    } type;

    std::variant<
        std::string,
        Keyword
    > data;
};

struct Lexer {
    FILE* in;
    const char* name;
    SourcePos pos = { this, 0, 0, 0 };
    char peekc = ' ';

    Token next() {
        char c = get();
        if(c == EOF) {
            return Token{Token::END};
        } else if(isalpha(c) || c == '_') {
            std::string buffer = {c};
            while(isalnum(peek()) || peek() == '_') {
                buffer += get();
            }
            return Token{Token::IDENT, buffer};
        } else {
            switch(c) {
            case ';': return Token{Token::KEY, Keyword::SEMICOLON};
            }
        }
    }

    bool consume(char c) {
        if(peek() == c) {
            get();
            return true;
        }
        return false;
    }

    char get() {
        char c = peekc;
        peekc = fgetc(in);
        if(c == '\n') {
            pos.col = 0;
            pos.line += 1;
        } else {
            pos.col += 1;
        }
        pos.dist += 1;
        return c;
    }

    char peek() {
        return peekc;
    }
};

struct Parser {
    Program parse() {

    }
};

int main(int argc, char** argv) {

}