#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void token_free(token_t* self)
{
    if(self->type & (tt_str | tt_ident))
        free(self->str);
}

int nextc(lexer_t* self)
{
    return self->file.next(self->file.data);
}

int peekc(lexer_t* self)
{
    return self->file.peek(self->file.data);
}

void push(lexer_t* self, char c)
{
    self->buf[self->top++] = c;
}   

void clear(lexer_t* self)
{
    self->top = 0;
}

inline token_t make_eof()
{
    token_t tok = { .type = tt_eof };
    return tok;
}

inline token_t make_string(lexer_t* self)
{
    int c = nextc(self);
    while(c != '"')
    {
        push(self, c);
        c = nextc(self);
    }

    token_t tok = {
        .type = tt_str,
        .str = strdup(self->buf)
    };

    return tok;
}

inline token_t make_char(lexer_t* self)
{
    token_t tok = {
        .type = tt_char,
        .letter = nextc(self)
    };

    nextc(self);

    return tok;
}

inline token_t make_number(lexer_t* self, int c)
{
    token_t tok = {
        .type = tt_int
    };

    return tok;
}

typedef struct { const char* name; keyword_e key; } keypair_t;

static const keypair_t keypairs[] = {
    { "using", kw_using },
    { "module", kw_module },
    { "import", kw_import },
    { "scope", kw_scope },
    { "def", kw_def },
    { "return", kw_return },
    { "let", kw_let },
    { "var", kw_var },
    { "while", kw_while },
    { "for", kw_for },
    { "match", kw_match },
    { "if", kw_else },
    { "else", kw_else },
    { "union", kw_union },
    { "enum", kw_enum }
};

static const size_t keypair_len = sizeof(keypairs) / sizeof(keypair_t);

inline token_t make_keyword(lexer_t* self, int c)
{
    push(self, c);
    c = peekc(self);
    while(isalnum(c) || c == '_')
    {
        push(self, nextc(self));
        c = peekc(self);
    }

    for(int i = 0; i < keypair_len; i++)
    {
        if(strcmp(self->buf, keypairs[i].name) == 0)
        {
            token_t tok = {
                .type = tt_keyword,
                .key = keypairs[i].key
            };

            return tok;
        }
    }

    token_t tok = {
        .type = tt_ident,
        .str = strdup(self->buf)
    };

    return tok;
}

token_t lexer_parse(lexer_t* self)
{
    clear(self);
    int c = nextc(self);

    while(isspace(c))
        c = nextc(self);

    if(c == '#')
        while(c != '\n')
            c = nextc(self);

    token_t tok = {
        .type = tt_keyword
    };

    switch(c)
    {
        // handle EOF
    case '\0': case -1:
        return make_eof();

        // handle string
    case '"':
        return make_string(self);

        // handle character
    case '\'':
        return make_char(self);

        // operators
    case '=':
        tok.key = peekc(self) == '=' ? op_eq : op_assign, nextc(self);
        return tok;
        
    case '!':
        tok.key = peekc(self) == '=' ? nextc(self), op_neq : op_not;
        return tok;

    case '~':
        tok.key = op_bitnot;
        return tok;

    case ',':
        tok.key = op_comma;
        return tok;

    case '+':
        tok.key = peekc(self) == '=' ? nextc(self), op_addeq : op_add;
        return tok;

    case ':':
        tok.key = peekc(self) == ':' ? nextc(self), op_sep : op_colon;
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
        nextc(self);
        return tok;

    case '/':
        tok.key = peekc(self) == '=' ? nextc(self), op_diveq : op_div;
        return tok;

    case '*':
        tok.key = peekc(self) == '=' ? nextc(self), op_muleq : op_mul;
        return tok;

    case '%':
        tok.key = peekc(self) == '=' ? nextc(self), op_modeq: op_mod;
        return tok;

    case '|':
        switch(peekc(self))
        {
        case '|':
            tok.key = op_or;
            break;
        case '=':
            tok.key = op_bitoreq;
            break;
        default:
            tok.key = op_bitor;
            return tok;
        }
        nextc(self);
        return tok;

    case '&':
        switch(peekc(self))
        {
        case '&':
            tok.key = op_and;
            break;
        case '=':
            tok.key = op_bitandeq;
            break;
        case '(':
            tok.key = op_func;
            break;
        default:
            tok.key = op_bitand;
            return tok;
        }
        nextc(self);
        return tok;

    case '^':
        tok.key = peekc(self) == '=' ? nextc(self), op_bitxoreq : op_bitxor;
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
        tok.key = op_closearr;
        return tok;

    case '<':
        switch(peekc(self))
        {
        case '<':
            nextc(self);
            tok.key = peekc(self) == '=' ? nextc(self), op_shleq : op_shl;
            return tok;
        case '=':
            tok.key = op_lte;
            break;
        default:
            tok.key = op_lt;
            return tok;
        }
        nextc(self);
        return tok;
    case '>':
        switch(peekc(self))
        {
        case '>':
            nextc(self);
            tok.key = peekc(self) == '=' ? nextc(self), op_shreq : op_shr;
            return tok;
        case '=':
            tok.key = op_gte;
            break;
        default:
            tok.key = op_gt;
            return tok;
        }
        nextc(self);
        return tok;
        // handle numbers
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        return make_number(self, c);

    default:
        return make_keyword(self, c);
    }
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

lexer_t lexer_alloc(file_t file)
{
    lexer_t lex = {
        .file = file,
        .top = 0,
        // TODO: make this configurable
        .buf = malloc(sizeof(int) * 512)
    };

    return lex;
}

void lexer_free(lexer_t* self)
{
    free(self->buf);
    self->file.close(self->file.data);
}

#if 0

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

#endif