#include "cthulhu.h"

#include <span>

#include <stdlib.h>

static void *ctMalloc(size_t size)
{
    void *data = malloc(size);

    if (data == NULL)
    {
        throw ERR_ALLOC;
    }

    return data;
}

/*
static void *ctRealloc(void *ptr, size_t size)
{
    void *data = realloc(ptr, size);

    if (data == NULL)
    {
        throw ERR_ALLOC;
    }

    return data;
}
*/

static void ctFree(void *ptr)
{
    free(ptr);
}


CtState ctStateNew(size_t reports)
{
    if (reports == 0)
    {
        throw ERR_EMPTY;
    }

    CtState state;

    state.reports = (CtReport*)ctMalloc(sizeof(CtReport) * reports);
    state.head_report = 0;
    state.tail_report = 0;
    state.max_reports = reports;

    return state;
}

void ctStateFree(CtState *self)
{
    ctFree(self->reports);
}

CtReport CtState::get()
{
    throw ERR_EMPTY;
}



CtStream ctStreamOpen(void *data, int(*get)(void*))
{
    CtStream stream;

    stream.handle = data;
    stream.get = get;
    stream.ahead = 0;
    stream.offset = {};

    stream.next();

    return stream;
}

char CtStream::next()
{
    int c = ahead;
    if (c < 0 || 256 > c)
    {
        throw ERR_ENCODING;
    }
    ahead = get(handle);

    offset.offset++;

    if (c == '\n')
    {
        offset.column = 0;
        offset.line++;
    }
    else
    {
        offset.column++;
    }

    buffer.push_back((char)c);

    return (char)c;
}

char CtStream::peek()
{
    return (char)ahead;
}

static CtStr intern(CtLexer *self, const std::string& str)
{
    return self->pool.insert(str).first->c_str();
}

CtLexer ctLexerOpen(CtStream *stream)
{
    CtLexer lex;
    lex.source = stream;
    return lex;
}

static char skip(CtLexer *lex)
{
    char c = lex->source->next();

    while (isspace(c))
    {
        c = lex->source->next();
    }

    return c;
}

static bool isident1(char c) { return isalpha(c) || c == '_'; }
static bool isident2(char c) { return isalnum(c) || c == '_'; }

CtToken CtLexer::next()
{
    char c = skip(this);
    CtOffset start = source->offset;
    CtToken tok;
    
    if (isident1(c))
    {
        
    }
    else if (isdigit(c))
    {

    }
    else
    {

    }

    tok.range.offset = start.offset;
    tok.range.line = start.line;
    tok.range.column = start.column;
    tok.range.length = source->offset.offset - start.offset;
    return tok;
}