#include "ctu.h"

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

void ctu_token_delete(ctu_token tok)
{
    if(tok.type == TOK_IDENT || tok.type == TOK_STRING)
        free(tok.string);
}


static ctu_token begin_tok(ctu_lexer* lex, ctu_tok_type type)
{
    ctu_token tok;
    tok.type = type;

    tok.pos.file = lex->buffer;
    tok.pos.distance = lex->distance;
    tok.pos.line = lex->line;
    tok.pos.col = lex->col;

    return tok;
}

static void end_tok(ctu_lexer* lex, ctu_token* tok)
{
    tok->length = lex->distance - tok->pos.distance;
}

static int peekc(ctu_lexer* lex)
{
    return lex->ch;
}

static int nextc(ctu_lexer* lex)
{
    int temp = lex->ch;
    lex->ch = lex->file.next(lex->file.handle);
    buffer_push(lex->buffer, temp);
    lex->distance += 1;

    if(temp == '\n')
    {
        lex->line += 1;
        lex->col = 0;
    }
    else
    {
        lex->col += 1;
    }

    return temp;
}

static int eatc(ctu_lexer* lex, int c)
{
    if(peekc(lex) == c)
    {
        nextc(lex);
        return 1;
    }
    return 0;
}

static int skip_whitespace(ctu_lexer* lex, int i)
{
    while(isspace(i))
        i = nextc(lex);

    return i;
}

static ctu_token lex_alpha(ctu_lexer* lex, int c)
{
    ctu_token tok = begin_tok(lex, TOK_KEY);
    buffer_t* buf = buffer_new();
    buffer_push(buf, c);

    while(isalnum(peekc(lex)) || peekc(lex) == '_')
        buffer_push(buf, nextc(lex));

#define KEY(id, str) if(strcmp(str, buf->data) == 0) { tok.keyword = id; goto finish_key; }
#define OP(id, str) if(strcmp(str, buf->data) == 0) { tok.keyword = id; goto finish_key; }
#define RES(id, str) if(strcmp(str, buf->data) == 0) { /* ERROR */ }
#include "keywords.inc"
    
    tok.type = TOK_IDENT;
    tok.string = buf->data;
    free(buf);

finish_key:
    end_tok(lex, &tok);
    return tok;
}

static ctu_token lex_digit(ctu_lexer* lex, int c)
{
    if(c == '0')
    {
        ctu_token tok = begin_tok(lex, TOK_INT);

        int i = nextc(lex);
        if(i == 'x')
        {
            buffer_t* buf = buffer_new();
            while(isxdigit(peekc(lex)))
                buffer_push(buf, nextc(lex));

            tok.integer = strtoll(buf->data, NULL, 16);
            end_tok(lex, &tok);

            return tok;
        }
        else if(i == 'b')
        {

        }
        else
        {
            // error
        }
    }
    buffer_t* buf = buffer_new();
    buffer_push(buf, c);
}

static ctu_token lex_string(ctu_lexer* lex, int c)
{

}

static ctu_token lex_char(ctu_lexer* lex, int c)
{

}

static ctu_token lex_symbol(ctu_lexer* lex, int c)
{
    ctu_token tok = begin_tok(lex, TOK_KEY);
    switch(c)
    {
    case ':':
        tok.keyword = eatc(lex, ':') ? kcolon2 : eatc(lex, '=') ? kassign : kcolon;
        break;
    case '=':
        if(eatc(lex, '='))
        {
            tok.keyword = keq;
        }
        else if(eatc(lex, '>'))
        {
            tok.keyword = kbigarrow;
        }
        else
        {
            // invalid keyword
        }
        break;
    case '<':
        if(eatc(lex, '<'))
        {
            if(eatc(lex, '='))
            {
                tok.keyword = kshleq;
            }
            tok.keyword = kshl;
        }
        else if(eatc(lex, '='))
        {
            tok.keyword = klte;
        }
        else
        {
            tok.keyword = klt;
        }
        break;
    case '>':
        if(eatc(lex, '>'))
        {
            if(eatc(lex, '='))
            {
                tok.keyword = kshreq;
            }
            tok.keyword = kshr;
        }
        else if(eatc(lex, '='))
        {
            tok.keyword = kgte;
        }
        else
        {
            tok.keyword = kgt;
        }
        break;
    case '!':
        if(eatc(lex, '='))
        {
            tok.keyword = kneq;
        }
        else
        {
            tok.keyword = knot;
        }
        break;
    case '|':
        if(eatc(lex, '='))
        {
            tok.keyword = kbitoreq;
        }
        else if(eatc(lex, '|'))
        {
            tok.keyword = kor;
        }
        else
        {   
            tok.keyword = kbitor;
        }
        break;
    case '&':
        if(eatc(lex, '='))
        {
            tok.keyword = kbitandeq;
        }
        else if(eatc(lex, '&'))
        {
            tok.keyword = kand;
        }
        else
        {
            tok.keyword = kbitand;
        }
        break;
    case '+':
        if(eatc(lex, '='))
        {
            tok.keyword = kaddeq;
        }
        else
        {
            tok.keyword = kadd;
        }
        break;
    case '-':
        if(eatc(lex, '='))
        {
            tok.keyword = ksubeq;
        }
        else
        {
            tok.keyword = ksub;
        }
        break;
    case '/':
        if(eatc(lex, '='))
        {
            tok.keyword = kdiveq;
        }
        else
        {
            tok.keyword = kdiv;
        }
        break;
    case '*':
        if(eatc(lex, '='))
        {
            tok.keyword = kmuleq;
        }
        else
        {
            tok.keyword = kmul;
        }
        break;
    case '%':
        if(eatc(lex, '='))
        {
            tok.keyword = kmodeq;
        }
        else
        {
            tok.keyword = kmod;
        }
        break;
    case '~':
        tok.keyword = kbitnot;
        break;
    case '^':
        if(eatc(lex, '='))
        {
            tok.keyword = kbitxoreq;
        }
        else
        {
            tok.keyword = kbitxor;
        }
        break;
    case '[':
        tok.keyword = klsquare;
        break;
    case ']':
        tok.keyword = krsquare;
        break;
    case '(':
        tok.keyword = klparen;
        break;
    case ')':
        tok.keyword = krparen;
        break;
    case '{':
        tok.keyword = klbrace;
        break;
    case '}':
        tok.keyword = krbrace;
        break;
    case '.':
        tok.keyword = kdot;
        break;
    case ',':
        tok.keyword = kcomma;
        break;
    case '?':
        tok.keyword = kquestion;
        break;
    default: 
        // invalid keyword
        break;
    }

    end_tok(lex, &tok);
    return tok;
}

static ctu_token ctu_lexer_parse(ctu_lexer* lex)
{
    int c = nextc(lex);

    c = skip_whitespace(lex, c);

    while(c == '#')
    {
        while(c != '\n')
            c = nextc(lex);

        c = skip_whitespace(lex, c);
    }

    if(c == EOF)
    {
        ctu_token tok = begin_tok(lex, TOK_EOF);
        end_tok(lex, &tok);
        return tok;
    }
    else if(isalpha(c) || c == '_')
    {
        return lex_alpha(lex, c);
    }
    else if(isdigit(c))
    {
        return lex_digit(lex, c);
    }
    else if(c == '"')
    {
        return lex_string(lex, c);
    }
    else if(c == '\'')
    {
        return lex_char(lex, c);
    }
    else
    {
        return lex_symbol(lex, c);
    }
}

ctu_lexer ctu_lexer_new(ctu_file file)
{
    ctu_lexer lex;
    lex.file = file;
    lex.ch = file.next(file.handle);

    lex.buffer = buffer_new();
    lex.col = 0;
    lex.distance = 0;
    lex.line = 0;
    
    lex.tok = ctu_lexer_parse(&lex);
    return lex;
}
