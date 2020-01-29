#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>

typedef struct {
    const char* keyword;
    keyword_t key;
} keyword_pair_t;

static const keyword_pair_t keypairs[] = {
    { "for", kw_for },
    { "while", kw_while },
    { "if", kw_if },
    { "else", kw_else },
    { "def", kw_def },
    { "switch", kw_switch },
    { "case", kw_case }
};

static const size_t keypair_len = sizeof(keypairs) / sizeof(keyword_pair_t);

static token_t make_keyword(lexer_t* self)
{
    token_t tok;
    tok.col = self->col - self->buf_cursor;
    tok.row = self->row;

    for(int i = 0; i < keypair_len; i++)
    {
        if(strcmp(self->buf, keypairs[i].keyword) == 0)
        {
            tok.type = keyword;
            tok.key = keypairs[i].key;
            return tok;
        }
    }

    tok.type = ident;
    tok.str = strdup(self->buf);
    printf("%s\n", self->buf);
    return tok;
}


static const keyword_pair_t operatorpairs[] = {
    { "=", op_assign },
    { "+", op_add },
    { "+=", op_addeq },
    { "-", op_sub },
    { "-=", op_subeq },
    { "/", op_div },
    { "/=", op_diveq },
    { "*", op_mul },
    { "*=", op_muleq },
    { "%", op_mod },
    { "%=", op_modeq }
};

static const size_t operator_len = sizeof(operatorpairs) / sizeof(keyword_pair_t);

static token_t make_operator(lexer_t* self)
{
    token_t tok;
    tok.col = self->col - self->buf_cursor;
    tok.row = self->row;

    for(int i = 0; i < operator_len; i++)
    {
        if(strcmp(self->buf, operatorpairs[i].keyword) == 0)
        {
            tok.type = keyword;
            tok.key = operatorpairs[i].key;
            return tok;
        }
    }

    // todo: error handling
    puts("invalid operator");
    exit(1);
}

static token_t make_string(lexer_t* self)
{
    token_t tok;
    tok.col = self->col - self->buf_cursor;
    tok.row = self->row;
    tok.type = string;
    tok.str = strdup(self->buf);
    return tok;
}

void token_free(token_t tok)
{
    if(tok.type == string)
        free(tok.str);
}

void lexer_free(lexer_t* self)
{
    self->file->close(self->file->data);
    free(self);
}

static char nextc(lexer_t* self)
{
    char c = self->file->next(self->file->data);
    
    while(isspace(c))
    {
        if(c == '\n')
        {
            self->col = 0;
            self->row++;
        }
        else
        {
            self->col++;
        }

        c = self->file->next(self->file->data);
    }

    return c;
}

static char peekc(lexer_t* self)
{
    return self->file->peek(self->file->data);
}

static void pushc(lexer_t* self, char c)
{
    self->buf[self->buf_cursor++] = c;
    self->buf[self->buf_cursor] = '\0';

    assert(self->buf_cursor < 2048);
}

static void clear_buf(lexer_t* self)
{
    self->buf[0] = '\0';
    self->buf_cursor = 0;
}

static token_t lexer_parse(lexer_t* self)
{
    token_t tok;
    char c = nextc(self);

    clear_buf(self);

    // first letter of keyword
    uint64_t cur = self->cursor;

    if(c == '\0')
    {
        token_t tok;
        tok.type = eof;
        return tok;
    }
    else if(c == '#')
    {
        while(nextc(self) != '\n') {}
        return lexer_next(self);
    }
    else if(isalpha(c) || c == '_')
    {
        pushc(self, c);
        for(;;)
        {
            c = peekc(self);
            if(isalnum(c) || c == '_')
            {
                pushc(self, nextc(self));
            }
            else
            {
                tok = make_keyword(self);
                break;
            }
        }
    }
    else if(isdigit(c))
    {
        for(;;)
        {
            pushc(self, c);

            c = peekc(self);

            if(isdigit(c) || c == '.')
            {
                pushc(self, nextc(self));
            }
        }
    }
    else if(c == '"')
    {
        for(;;)
        {
            c = nextc(self);

            if(c != '"')
            {
                pushc(self, c);
            }
            else
            {
                tok = make_string(self);
                break;
            }
        }
    }
    else
    {
        pushc(self, c);
        for(;;)
        {
            c = peekc(self);
            if(isspace(c) || isalnum(c))
            {
                pushc(self, nextc(self));
            }
            else
            {
                tok = make_operator(self);
                break;
            }
        }
    }

    tok.cursor = cur;
    return tok;
}

lexer_t* lexer_alloc(file_t* file)
{
    lexer_t* lex = malloc(sizeof(lexer_t));
    
    lex->col = 0;
    lex->row = 0;
    lex->file = file;
    lex->cursor = 0;
    lex->buf = malloc(sizeof(char) * 2048);
    lex->buf_cursor = 0;
    lex->buf[lex->buf_cursor] = '\0';

    lex->tok = lexer_parse(lex);

    return lex;
}

token_t lexer_next(lexer_t* self)
{
    token_t temp = self->tok;
    self->tok = lexer_parse(self);
    return temp;
}

token_t lexer_peek(lexer_t* self)
{
    return self->tok;
}