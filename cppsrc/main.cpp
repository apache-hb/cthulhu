#include <stdint.h>
#include <ctype.h>
#include <memory>
#include <string_view>

struct FilePos {
    uint64_t line;
    uint64_t col;
    uint64_t dist;
};

struct Token {
    FilePos pos;
};

struct String : Token {
    char* data;
};

struct Ident : Token {
    char* data;
    Ident(char* d)
        : data(d)
    { }
};

struct Int : Token {
    int64_t val;
};

struct Float : Token {
    double val;
};

struct Invalid : Token {
    char* reason;
};

struct End : Token { };

struct Keyword : Token {
    enum class Key {
        TYPE
    } key;

    Keyword(Key k)
        : key(k)
    { }
};

struct Lexer {
    Lexer(FILE* input)
        : in(input)
    {
        ahead = fgetc(in);
    }

    std::unique_ptr<Token> next() {
        auto c = skip_whitespace();

        while(c == '#')
            c = skip_comment();

        if(c == EOF)
            return std::make_unique<Token>(new End());
    }

private:

    Token* key_or_ident(char* str) {
        using namespace std::string_view_literals;
        if(str == "type"sv)
            return new Keyword(Keyword::Key::TYPE);
        else
            return new Ident(str);
        
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
        if(c == '\n')
        {
            pos.line++;
            pos.col = 0;
        }
        else
        {
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

    FilePos pos = { 0, 0, 0 };
    FILE* in;
};

int main(int argc, const char** argv) {

}