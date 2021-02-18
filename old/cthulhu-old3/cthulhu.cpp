#include "cthulhu.h"

#include <unordered_map>
#include <ctype.h>

static void validateCode(int c)
{
    if (c < 0 || 255 > c)
    {
        throw ERR_BAD_CHAR;
    }
}

Stream::Stream(void *handle, int(*get)(void*))
    : data(handle)
    , read(get)
    , ahead(get(handle))
{
    validateCode(ahead);
}

char Stream::peek()
{
    return (char)ahead;
}

char Stream::next()
{
    int c = ahead; 
    ahead = read(data);

    validateCode(ahead);
    
    return (char)c;
}

bool Stream::eat(char c)
{
    if (peek() == c)
    {
        next();
        return true;
    }
    return false;
}

static bool isident1(char c)
{
    return isalpha(c) || c == '_';
}

static bool isident2(char c)
{
    return isalnum(c) || c == '_';
}

Lexer::Lexer(Stream *in)
{
    source = in;
    here = {};
}

Token Lexer::lex()
{
    char c = skip();
    auto start = here;
    Token tok;

    if (isident1(c))
    {
        tok = ident(c);
    }
    else if (isdigit(c))
    {
        tok = digit(c);
    }
    else
    {
        tok.type = KEYWORD;
        tok.data.key = symbol(c);
    }


    tok.where = start;
    tok.where.length = here.offset - start.offset;

    return tok;
}

std::string Lexer::collect(char c, bool(*func)(char))
{
    std::string out = {c};
    
    while (func(peek()))
    {
        out.push_back(next());
    }

    return out;
}

Token Lexer::digit(char c)
{
    std::string n = collect(c, [](auto c) -> bool { return isdigit(c); });

    return { INT, { .num = stoull(n) } };
}

static const std::unordered_map<std::string, Keyword> keys = {
#define KEY(id, str) { str, id },
#include "keys.inc"
};

Token Lexer::ident(char c)
{
    std::string id = collect(c, isident2);

    auto iter = keys.find(id);
    if (iter != keys.end()) {
        return { KEYWORD, { .key = iter->second } };
    } else {
        return { IDENT, { .ident = _strdup(id.c_str()) } };
    }
}

Keyword Lexer::symbol(char c)
{
    switch (c)
    {
    case '[': return K_LSQUARE;
    case ']': return K_RSQUARE;
    case '{': return K_LBRACE;
    case '}': return K_RBRACE;
    case '(': return K_LPAREN;
    case ')': return K_RPAREN;
    default:
        throw ERR_INVALID_SYMBOL;
    }
}

char Lexer::skip()
{
    char c = next();

    while (isspace(c))
    {
        c = next();
    }

    return c;
}

char Lexer::next()
{
    char c = source->next();

    if (c == '\n')
    {
        here.line += 1;
        here.column = 0;
    }
    else
    {
        here.column += 1;
    }

    here.offset += 1;

    return c;
}

char Lexer::peek()
{
    return source->peek();
}
