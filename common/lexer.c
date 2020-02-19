#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void token_free(ctu_token* self)
{
    if(self->type & (tt_str | tt_ident))
        free(self->str);
}

int nextc(ctu_lexer* self)
{
    return self->file.next(self->file.data);
}

int peekc(ctu_lexer* self)
{
    return self->file.peek(self->file.data);
}

void push(ctu_lexer* self, char c)
{
    self->buf[self->top++] = c;
    self->buf[self->top] = '\0';
}   

void clear(ctu_lexer* self)
{
    self->top = 0;
}

inline ctu_token make_eof()
{
    ctu_token tok = { .type = tt_eof };
    return tok;
}

inline ctu_token make_string(ctu_lexer* self)
{
    int c = nextc(self);
    while(c != '"')
    {
        push(self, c);
        c = nextc(self);
    }

    ctu_token tok = {
        .type = tt_str,
        .str = strdup(self->buf)
    };

    return tok;
}

inline ctu_token make_char(ctu_lexer* self)
{
    ctu_token tok = {
        .type = tt_char,
        .letter = nextc(self)
    };

    nextc(self);

    return tok;
}

inline ctu_token make_number(ctu_lexer* self, int c)
{
    ctu_token tok = {
        .type = tt_int
    };

    return tok;
}

typedef struct { const char* name; keyword_e key; } ctu_keypair;

static const ctu_keypair keypairs[] = {
    { "using", kw_using },
    { "module", kw_module },
    { "import", kw_import },
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

static const size_t keypair_len = sizeof(keypairs) / sizeof(ctu_keypair);

inline ctu_token make_keyword(ctu_lexer* self, int c)
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
            ctu_token tok = {
                .type = tt_keyword,
                .key = keypairs[i].key
            };

            return tok;
        }
    }

    ctu_token tok = {
        .type = tt_ident,
        .str = strdup(self->buf)
    };

    return tok;
}

ctu_token lexer_parse(ctu_lexer* self)
{
    clear(self);
    int c = nextc(self);

    while(isspace(c))
        c = nextc(self);

    if(c == '#')
        while(c != '\n')
            c = nextc(self);

    ctu_token tok = {
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

ctu_token lexer_next(ctu_lexer* self)
{
    ctu_token temp = self->tok;
    self->tok = lexer_parse(self);
    return temp;
}

ctu_token lexer_peek(ctu_lexer* self)
{
    return self->tok;
}

ctu_lexer lexer_alloc(ctu_file file)
{
    ctu_lexer lex = {
        .file = file,
        .top = 0,
        // TODO: make this configurable
        .buf = malloc(sizeof(int) * 512)
    };

    lex.tok = lexer_parse(&lex);

    return lex;
}

void lexer_free(ctu_lexer* self)
{
    free(self->buf);
    self->file.close(self->file.data);
}
