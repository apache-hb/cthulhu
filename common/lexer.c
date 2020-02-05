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
    { "case", kw_case },
    { "scope", kw_scope },
    { "return", kw_return },
    { "using", kw_using },
    { "val", kw_let },
    { "var", kw_var },
    { "union", kw_union },
    { "enum", kw_enum }
};

static const size_t keypair_len = sizeof(keypairs) / sizeof(keyword_pair_t);

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

static char next(lexer_t* self)
{
    char c = self->file->next(self->file->data);
    
    if(c == '\n')
    {
        self->col = 0;
        self->row++;
    }
    else
    {
        self->col++;
    }

    return c;
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
    token_t tok = { keyword };
    char c = nextc(self);

    clear_buf(self);

    // first letter of keyword
    uint64_t cur = self->cursor;

    keyword_t hint = kw_none;

    switch(c)
    {
        // handle eof
    case -1: case '\0':
        tok.type = eof;
        return tok;

        // skip comments
    case '#':
        while((c = next(self)))
        {
            if(c == '\n')
            {
                break;
            }
            else if(c == -1 || c == '\0')
            {
                tok.type = eof;
                return tok;
            }
        }
        return lexer_parse(self);

        // strings
    case '"':
        while((c = next(self)))
        {
            if(c == '"')
            {
                tok.type = string;
                tok.str = strdup(self->buf);
                return tok;
            }
            else
            {
                pushc(self, c);
            }
        }

    case '=':
        tok.key = peekc(self) == '=' ? op_eq : op_assign, next(self);
        return tok;

    case '+':
        tok.key = peekc(self) == '=' ? op_addeq : op_add, next(self);
        return tok;

    case '-':
        switch(peekc(self))
        {
        case '=':
            tok.key = op_subeq;
            break;
        case '>':
            tok.key = op_arrow;
            break;
        default:
            tok.key = op_sub;
            return tok;
        }
        next(self);
        return tok;

    case '/':
        tok.key = peekc(self) == '=' ? op_diveq : op_div, next(self);
        return tok;

    case '*':
        tok.key = peekc(self) == '=' ? op_muleq : op_mul, next(self);
        return tok;

    case '%':
        tok.key = peekc(self) == '=' ? op_modeq : op_mod, next(self);
        return tok;

    case '|':
        switch(peekc(self))
        {
        case '|':
            tok.key = op_or;
            break;
        case '=':
            tok.key = op_bitor;
            break;
        default:
            tok.key = op_or;
            break;
        }
        return tok;
    
    case '&':
        switch(peekc(self))
        {
        case '(':
            tok.key = op_func;
            break;
        case '&':
            tok.key = op_and;
            break;
        case '=':
            tok.key = op_bitandeq;
            break;
        default:
            tok.key = op_bitand;
            break;
        }
        return tok;

    case '^':
        tok.key = peekc(self) == '=' ? op_bitxoreq : op_bitxor, next(self);
        return tok;

    case '~':
        tok.key = op_bitnot;
        return tok;

    case '!':
        tok.key = peekc(self) == '=' ? op_neq : op_not, next(self);
        return tok;

    case ':':
        tok.key = peekc(self) == ':' ? op_sep : op_colon, next(self);
        return tok;

    case '(':
        tok.key = op_openarg;
        return tok;

    case ')':
        tok.key = op_closearg;
        return tok;

    case '{':
        tok.key = op_openscope;
        return tok;

    case '}':
        tok.key = op_closescope;
        return tok;

    case '[':
        tok.key = op_openarr;
        return tok;

    case ']':
        tok.key = op_closescope;
        return tok;

    case ',':
        tok.key = op_comma;
        return tok;

    case '>':
        switch(peekc(self))
        {
        case '=':
            tok.key = op_gte;
            break;
        case '>':
            next(self);
            tok.key = peekc(self) == '=' ? op_shreq : op_shr;
            break;
        default:
            tok.key = op_gt;
            return tok;
        }
        next(self);
        return tok;
    case '<':
        switch(peekc(self))
        {
        case '=':
            tok.key = op_lte;
            break;
        case '<':
            next(self);
            tok.key = peekc(self) == '=' ? op_shleq : op_shl;
            break;
        default:
            tok.key = op_lt;
            return tok;
        }
        next(self);
        return tok;

        // numbers
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
    case '8': case '9':
        // TODO: parse numbers
        break;

    default:
        pushc(self, c);
        while((c = peekc(self)))
        {
            if(isalnum(c) || c == '_')
            {
                pushc(self, next(self));
            }
            else
            {
                for(int i = 0; i < keypair_len; i++)
                {
                    if(strcmp(keypairs[i].keyword, self->buf) == 0)
                    {
                        tok.key = keypairs[i].key;
                        return tok;
                    }
                }

                tok.type = ident;
                tok.str = strdup(self->buf);
                return tok;
            }
        }
        
        // parse idents and text keywords
        break;
    }

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