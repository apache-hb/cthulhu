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

    tok.pos.file = &lex->buffer;
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
    vec_push(&lex->buffer, temp);
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
    vec_char_t buf;
    vec_init(&buf);
    vec_push(&buf, c);

    while(isalnum(peekc(lex)) || peekc(lex) == '_')
        vec_push(&buf, nextc(lex));

    vec_push(&buf, '\0');

    printf("buffer = %s\n", buf.data);

#define KEY(id, str) if(strcmp(str, buf.data) == 0) { tok.keyword = id; goto finish_key; }
#define OP(id, str) if(strcmp(str, buf.data) == 0) { tok.keyword = id; goto finish_key; }
#define RES(id, str) if(strcmp(str, buf.data) == 0) { /* ERROR */ }
#include "keywords.inc"
    
    tok.type = TOK_IDENT;
    tok.string = buf.data;
    end_tok(lex, &tok);
    return tok;

finish_key:
    end_tok(lex, &tok);
    vec_deinit(&buf);
    return tok;
}

static ctu_token lex_digit(ctu_lexer* lex, int c)
{
    ctu_token tok = begin_tok(lex, TOK_INT);
    if(c == '0')
    {
        vec_char_t buf;
        vec_init(&buf);

        int i = nextc(lex);
        if(i == 'x')
        {
            while(isxdigit(peekc(lex)))
                vec_push(&buf, nextc(lex));

            vec_push(&buf, '\0');
            tok.integer = strtoll(buf.data, NULL, 16);
        }
        else if(i == 'b')
        {
            while(peekc(lex) == '1' || peekc(lex) == '0')
                vec_push(&buf, nextc(lex));

            vec_push(&buf, '\0');
            tok.integer = strtoll(buf.data, NULL, 2);
        }
        else
        {
            /* error */
        }

        vec_deinit(&buf);
        end_tok(lex, &tok);
        return tok;
    }

    vec_char_t buf;
    vec_init(&buf);

    int isdecimal = 0;

    while(peekc(lex) == '.' || isdigit(peekc(lex)))
    {
        if(peekc(lex) == '.')
        {
            if(isdecimal)
            {
                /* error */
            }

            isdecimal = 1;
        }
        vec_push(&buf, nextc(lex));
    }

    vec_push(&buf, '\0');

    if(isdecimal)
    {
        tok.type = TOK_FLOAT;
        tok.number = strtod(buf.data, NULL);
    }
    else
    {
        tok.integer = strtoll(buf.data, NULL, 10);
    }
    vec_deinit(&buf);
    end_tok(lex, &tok);
    return tok;
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
        else if(eatc(lex, '('))
        {
            tok.keyword = kfuncsig;
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

    vec_init(&lex.buffer);
    lex.col = 0;
    lex.distance = 0;
    lex.line = 0;
    
    lex.tok = ctu_lexer_parse(&lex);
    return lex;
}

ctu_token ctu_lexer_peek(ctu_lexer* lex)
{
    return lex->tok;
}

ctu_token ctu_lexer_next(ctu_lexer* lex)
{
    ctu_token tok = lex->tok;
    lex->tok = ctu_lexer_parse(lex);
    return tok;
}


ctu_parser ctu_parser_new(ctu_lexer* lex)
{
    ctu_parser parse = {
        .lex = lex,
        .preamble = 1
    };

    return parse;
}


static ctu_token nextt(ctu_parser* parse)
{
    return ctu_lexer_next(parse->lex);
}

static ctu_token peekt(ctu_parser* parse)
{
    return ctu_lexer_peek(parse->lex);
}

static int eatk(ctu_parser* parse, ctu_keyword key)
{
    if(peekt(parse).type == TOK_KEY && peekt(parse).keyword == key)
    {
        ctu_token_delete(nextt(parse));
        return 1;
    }
    return 0;
}

static ctu_token nextk(ctu_parser* parse)
{
    ctu_token tok = nextt(parse);
    if(tok.type != TOK_KEY)
    {
        /* error */
    }

    return tok;
}

static ctu_token nexti(ctu_parser* parse)
{
    ctu_token tok = nextt(parse);
    if(tok.type != TOK_IDENT)
    {
        /* error */
    }

    return tok;
}

static void expectk(ctu_parser* parse, ctu_keyword key)
{
    ctu_token tok = nextk(parse);
    if(tok.keyword != key)
    {
        /* error */
    }
}

static vec_str_t dotted_name(ctu_parser* parse)
{
    vec_str_t path;
    vec_init(&path);

    for(;;)
    {
        ctu_token tok = nextt(parse);
        if(tok.type != TOK_IDENT)
        {
            /* error */
        }
        vec_push(&path, tok.string);

        if(!eatk(parse, kcolon2))
            break;
    }

    return path;
}

static ctu_node parse_import(ctu_parser* parse)
{
    ctu_node node;
    node.i_path = dotted_name(parse);
    return node;
}

static ctu_node* parse_type(ctu_parser* parse)
{
    ctu_token tok = nextt(parse);
    if(tok.type == TOK_IDENT)
    {
        /* either typename or builtin */
        switch(tok.keyword)
        {
        case klbrace: return parse_struct(parse);
        case klparen: return parse_tuple(parse);
        case klsquare: return parse_array(parse);
        case kenum: return parse_enum(parse);
        case kvariant: return parse_variant(parse);
        case kunion: return parse_union(parse);
        default: /* error */ break;
        }
    }
    else if(tok.type == TOK_KEY)
    {
        return parse_builtin(parse);
        /* keyword */
    }
    else
    {
        return NULL;
        /* error */
    }
}

static ctu_node parse_typedef(ctu_parser* parse)
{
    ctu_node node;

    node.td_name = nexti(parse).string;

    expectk(parse, kassign);

    node.td_type = parse_type(parse);

    return node;
}

ctu_node ctu_parser_next(ctu_parser* parse)
{
    ctu_token tok = nextk(parse);

    if(tok.keyword == kimport)
    {
        if(!parse->preamble)
        {
            /* preamble is over, error */
        }

        return parse_import(parse);
    }

    parse->preamble = 0;

    if(tok.keyword == ktype) 
    {
        return parse_typedef(parse);
    }
    else if(tok.keyword == kdef)
    {

    }
    else if(tok.keyword == kscope)
    {

    }
    else
    {
        /* error */
    }
}
