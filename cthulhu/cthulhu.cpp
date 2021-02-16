#include "cthulhu.h"

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
    return ahead;
}

char Stream::next()
{
    int c = ahead; 
    ahead = read(data);

    validateCode(ahead);
    
    return c;
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
    else
    {
        tok = symbol(c);
    }


    tok.where = start;
    tok.where.length = here.offset - start.offset;

    return tok;
}

Token Lexer::ident(char c)
{

}

Token Lexer::symbol(char c)
{
    
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
