#include "lexer.h"

#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char* strdup(const char* str)
{
    size_t size = strlen(str);
    char* out = malloc(size+1);
    memcpy(out, str, size);
    out[size] = 0;
    return out;
}

lexer_t lexer_new(stream_t source)
{
    lexer_t out = {
        .source = source,
        .buffer = { 0 },
        .idx = 0
    };

    return out;
}

static void buffer_push(lexer_t* self, int c)
{
    self->buffer[self->idx] = c;
    self->buffer[self->idx] = '\0';
}

static void buffer_wipe(lexer_t* self)
{
    self->idx = 0;
    self->buffer[0] = '\0';
}

static int skip_whitespace(lexer_t* self)
{
    int c = stream_next(&self->source);
    while(isspace(c))
    {
        c = stream_next(&self->source);
    }

    return c;
}

static token_t make_eof()
{
    token_t tok = {
        .type = END
    };

    return tok;
}

static token_t make_error(char* msg)
{
    token_t tok = {
        .type = ERROR,
        .data.error = msg
    };

    return tok;
}

static token_t keyword_or_ident(char* ident)
{
    token_t tok = {
        .type = KEYWORD
    };

#define KEY(id, str) if(strcmp(str, ident) == 0) { tok.data.key = id; return tok; }
#include "keywords.inc"

    tok.type = IDENT;
    tok.data.ident = strdup(ident);
    return tok;
}

token_t lexer_next(lexer_t* self)
{
    int c = skip_whitespace(self);
    streampos_t here;
    token_t tok;

    while(c == '#')
    {
        while(c != '\n')
        {
            c = stream_next(&self->source);
        }

        c = skip_whitespace(self);
    }

    here = self->source.pos;

    if(isalpha(c) || c == '_')
    {
        buffer_wipe(self);
        buffer_push(self, c);
        
        for(;;) 
        {
            c = stream_peek(&self->source);
            if(isalnum(c) || c == '_')
            {
                buffer_push(self, stream_next(&self->source));
            }
            else
            {
                break;
            }
        }

        tok = keyword_or_ident(self->buffer);
    }
    else
    {
        switch(c)
        {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '!':
        case '&':
        case '|':
        case '@':
        case '~':
        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case '<':
        case '>':
        case ':':
        case '=':
        case '?':
        case '.':
        case ',':
        case EOF:
            tok = make_eof();
            break;
        default:
            tok = make_error(strfmt("`%c` is an illegal character", c));
            break;
        }
    }

    tok.pos = here;
    return tok;
}
