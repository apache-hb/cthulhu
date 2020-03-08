#include "ctu.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

void ctu_free_token(ctu_token tok)
{
    if(tok.type == ctu_tt_str || tok.type == ctu_tt_ident)
        free(tok.str);
}

static ctu_token str_tok(char* str)
{
    ctu_token tok;

    tok.type = ctu_tt_str;
    tok.str = str;

    return tok;
}

static ctu_token ident_tok(char* ident)
{
    ctu_token tok;
    
    tok.type = ctu_tt_ident;
    tok.ident = ident;

    return tok;
}

static ctu_token char_tok(char ch)
{
    ctu_token tok;
    
    tok.type = ctu_tt_char;
    tok.ch = ch;

    return tok;
}

static ctu_token int_tok(int64_t num)
{
    ctu_token tok;
    
    tok.type = ctu_tt_int;
    tok.integer = num;

    return tok;
}

static ctu_token num_tok(double num)
{
    ctu_token tok;
    
    tok.type = ctu_tt_num;
    tok.number = num;

    return tok;
}

static ctu_token key_tok(ctu_keyword key)
{
    ctu_token tok;
    
    tok.type = ctu_tt_key;
    tok.key = key;

    return tok;
}

static ctu_token eof_tok()
{
    ctu_token tok;
    
    tok.type = ctu_tt_eof;

    return tok;
}

static int nextc(ctu_lexer* lex)
{
    int i = lex->input.next(lex->input.handle);

    lex->pos++;

    if(i == '\n')
    {
        lex->line++;
        lex->col = 0;
    }
    else
    {
        lex->col++;
    }

    return i;
}

static int peekc(ctu_lexer* lex)
{
    return lex->input.peek(lex->input.handle);
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

static int ctu_skip_whitespace(ctu_lexer* lex, int i)
{
    while(isspace(i))
        i = nextc(lex);

    return i;
}

static ctu_token ctu_lexer_parse_digit(ctu_lexer* lex, int i)
{
    vec_char_t vec;
    vec_init(&vec);

    if(i == '0')
    {
        nextc(lex);

        /* handle hex number */
        if(peekc(lex) == 'x')
        {
            while(isxdigit(peekc(lex)))
                vec_push(&vec, nextc(lex));

            vec_push(&vec, 0);

            ctu_token tok = int_tok(strtol(vec.data, NULL, 16));
            vec_deinit(&vec);
            return tok;
        }
        /* handle binary numbers */
        else if(peekc(lex) == 'b')
        {
            while(peekc(lex) == '0' || peekc(lex) == '1')
                vec_push(&vec, nextc(lex));

            vec_push(&vec, 0);

            ctu_token tok = int_tok(strtol(vec.data, NULL, 2));
            vec_deinit(&vec);
            return tok;
        }

        /* error: only hex and binary numbers can begin with 0 */
    }

    vec_push(&vec, i);

    int number = 0;

    while(isdigit(peekc(lex)) || peekc(lex) == '.')
    {
        if(peekc(lex) == '.')
            number = 1;

        vec_push(&vec, nextc(lex));
    }

    ctu_token tok = number ? num_tok(strtod(vec.data, NULL)) : int_tok(strtol(vec.data, NULL, 10));

    vec_deinit(&vec);

    return tok;
}

static ctu_token ctu_lexer_parse_alpha(ctu_lexer* lex, int i)
{
    vec_char_t vec;
    vec_init(&vec);
    vec_push(&vec, i);

    while(isalnum(peekc(lex)) || peekc(lex) == '_')
    {
        vec_push(&vec, nextc(lex));
    }

    vec_push(&vec, 0);

#define KEYWORD(id, str) if(strcmp(vec.data, str) == 0) { vec_deinit(&vec); return key_tok(id); }

#include "keywords.inc"

    return ident_tok(vec.data);
}

static ctu_token ctu_lexer_parse_str(ctu_lexer* lex)
{
    return str_tok("");
    /* todo */
}

static ctu_token ctu_lexer_parse_char(ctu_lexer* lex)
{
    return char_tok(' ');
    /* todo */
}

static ctu_token ctu_lexer_parse_symbol(ctu_lexer* lex, int i)
{
    switch(i)
    {
    case '~':
        return key_tok(kbitnot);
    case '!':
        return key_tok(eatc(lex, '=') ? kneq : knot);
    case '%':
        return key_tok(eatc(lex, '=') ? kmodeq : kmod);
    case '^':
        return key_tok(eatc(lex, '=') ? kbitxoreq : kbitxor);
    case '&':
        return key_tok(eatc(lex, '=') ? kbitandeq : eatc(lex, '&') ? kand : kbitand);
    case '*':
        return key_tok(eatc(lex, '=') ? kmuleq : kmul);
    case '(':
        return key_tok(klparen);
    case ')':
        return key_tok(krparen);
    case '-':
        return key_tok(eatc(lex, '=') ? ksubeq : eatc(lex, '>') ? karrow : ksub);
    case '=':
        return key_tok(keq);
    case '+':
        return key_tok(eatc(lex, '=') ? kaddeq : kadd);
    case '[':
        return key_tok(eatc(lex, '[') ? klsquare2 : klsquare);
    case ']':
        return key_tok(eatc(lex, ']') ? krsquare2 : krsquare);
    case '{':
        return key_tok(klbrace);
    case '}':
        return key_tok(krbrace);
    case '|':
        return key_tok(eatc(lex, '=') ? kbitoreq : eatc(lex, '|') ? kor : kbitor);
    case '<':
        if(eatc(lex, '<'))
        {
            return key_tok(eatc(lex, '=') ? kshleq : kshl);
        }

        return key_tok(eatc(lex, '=') ? klte : klt);
    case '>':
        if(eatc(lex, '>'))
        {
            return key_tok(eatc(lex, '=') ? kshreq : kshr);
        }

        return key_tok(eatc(lex, '=') ? kgte : kgt);
    case ',':
        return key_tok(kcomma);
    case '.':
        return key_tok(kdot);
    case '/':
        return key_tok(eatc(lex, '=') ? kdiveq : kdiv);
    case ':':
        return key_tok(eatc(lex, '=') ? kassign : eatc(lex, ':') ? kcolon2 : kcolon);        
    default:
        return key_tok(kinvalid);
        /* error: invalid symbol */
        break;
    }
}

static ctu_token ctu_lexer_parse(ctu_lexer* lex)
{
    int i = nextc(lex);

    /* check for eof */
    if(i == EOF)
        return eof_tok();

    i = ctu_skip_whitespace(lex, i);

    /* handle all comments */
    while(i == '#')
    {
        while(i != '\n')
            i = nextc(lex);

        i = ctu_skip_whitespace(lex, i);
    }

    if(isdigit(i))
    {
        return ctu_lexer_parse_digit(lex, i);
    }
    else if(isalpha(i) || i == '_')
    {
        return ctu_lexer_parse_alpha(lex, i);
    }
    else if(i == '"')
    {
        return ctu_lexer_parse_str(lex);
    }
    else if(i == '\'')
    {
        return ctu_lexer_parse_char(lex);
    }
    else
    {
        return ctu_lexer_parse_symbol(lex, i);
    }
}

ctu_lexer ctu_new_lexer(ctu_input input)
{
    ctu_lexer lexer;
    lexer.input = input;

    lexer.tok = ctu_lexer_parse(&lexer);

    return lexer;
}

void ctu_free_lexer(ctu_lexer* lexer)
{
    ctu_free_token(lexer->tok);
}

ctu_token ctu_lexer_next(ctu_lexer* lexer)
{
    ctu_token temp = lexer->tok;
    lexer->tok = ctu_lexer_parse(lexer);
    return temp;
}

ctu_token ctu_lexer_peek(ctu_lexer* lexer)
{
    return lexer->tok;
}

const char* ctu_key_str(ctu_keyword key)
{
#define OPERATOR(id, str) case id: return str;
#define KEYWORD(id, str) case id: return str;
#define ASM_KEYWORD(id, str) case id: return str;
    switch(key)
    {
#include "keywords.inc"
    default: return "invalid";
    }
}
